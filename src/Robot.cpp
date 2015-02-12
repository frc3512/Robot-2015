#include <Subsystems/ElevatorAutomatic.hpp>
#include "Robot.hpp"
#include <cmath>
#include "DriverStation.h"

Robot::Robot() : settings("/home/lvuser/RobotSettings.txt"),
                 drive1Buttons(0),
                 drive2Buttons(1),
                 elevatorButtons(2),
                 dsDisplay(DriverStationDisplay::getInstance(
                               settings.getInt("DS_Port"))),
                 insight(Insight::getInstance(settings.getInt("Insight_Port"))),
                 pidGraph(3513) {
    robotDrive = std::make_unique<DriveTrain>();

    driveStick1 = std::make_unique<Joystick>(0);
    driveStick2 = std::make_unique<Joystick>(1);
    shootStick = std::make_unique<Joystick>(2);
    autonTimer = std::make_unique<Timer>();
    displayTimer = std::make_unique<Timer>();
    ev = std::make_unique<ElevatorAutomatic>();

    dsDisplay.addAutonMethod("MotionProfile",
                             &Robot::AutonMotionProfile,
                             this);
    dsDisplay.addAutonMethod("Noop Auton", &Robot::NoopAuton, this);

    pidGraph.setSendInterval(200);

    displayTimer->Start();
}

Robot::~Robot() {
}

void Robot::OperatorControl() {
    robotDrive->reloadPID();

    while (IsEnabled() && IsOperatorControl()) {
        // arcade Drive
        if (driveStick2->GetRawButton(2)) {
            robotDrive->drive(driveStick1->GetY(), driveStick2->GetX(),
                              true);
        }
        else {
            robotDrive->drive(driveStick1->GetY(), driveStick2->GetX());
        }

        /* Manual state machine */
        if (elevatorButtons.releasedButton(2)) {
            ev->setManualMode(!ev->getManualMode());
        }

        /* Automatic preset buttons (7-12) */
        if (elevatorButtons.releasedButton(7)) {
            ev->setHeight(0);
        }
        if (elevatorButtons.releasedButton(8)) {
            ev->setHeight(0.5);
        }

        /* Print the height for debugging */
        //std::cout << "Height=" << ev->getHeight() << std::endl;

        /* Set manual value */
        ev->setManualLiftSpeed(shootStick->GetY());

        /* Trailing edge of trigger press */
        if (elevatorButtons.releasedButton(1)) {
            ev->elevatorGrab(!ev->getElevatorGrab());
        }
        if (elevatorButtons.releasedButton(5)) {
            ev->intakeGrab(!ev->getIntakeGrab());
        }
        if (elevatorButtons.releasedButton(6)) {
            ev->stowIntake(!ev->isIntakeStowed());
        }

        if (shootStick->GetPOV() == 180) {
            ev->setIntakeDirection(Elevator::S_FORWARD);
        }
        else if (shootStick->GetPOV() == 0) {
            ev->setIntakeDirection(Elevator::S_REVERSED);
        }
        else {
            ev->setIntakeDirection(Elevator::S_STOPPED);
        }

        // Opens intake if the elevator is at the same level as it
        if (ev->getHeight() < 11 && !ev->getIntakeGrab()) {
            ev->intakeGrab(true);
        }

        drive1Buttons.updateButtons();
        drive2Buttons.updateButtons();
        elevatorButtons.updateButtons();

        DS_PrintOut();

        Wait(0.01);
    }
}


void Robot::Autonomous() {
    autonTimer->Reset();
    autonTimer->Start();

    dsDisplay.execAutonomous();

    autonTimer->Stop();
}

void Robot::Disabled() {
    while (IsDisabled()) {
        DS_PrintOut();
        Wait(0.1);
    }
}

void Robot::DS_PrintOut() {
    if (pidGraph.hasIntervalPassed()) {
        pidGraph.graphData(ev->getHeight(), "Distance (EV)");
        pidGraph.graphData(ev->getSetpoint(), "Setpoint (EV)");

        pidGraph.resetInterval();
    }

    if (displayTimer->HasPeriodPassed(0.5)) {
        dsDisplay.clear();

        dsDisplay.addElementData("EV_HEIGHT", ev->getHeight());
        dsDisplay.addElementData("EV_SETPOINT", ev->getSetpoint());

        std::cout << "EV_HEIGHT=" << ev->getHeight() << std::endl;

        dsDisplay.sendToDS();
    }

    dsDisplay.receiveFromDS();
    insight.receiveFromDS();
}

START_ROBOT_CLASS(Robot);

