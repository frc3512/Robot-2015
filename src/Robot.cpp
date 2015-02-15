#include <Subsystems/ElevatorAutomatic.hpp>
#include "Robot.hpp"
#include <cmath>
#include "DriverStation.h"
#include <iostream>
#include <iomanip>

Robot::Robot() : settings("/home/lvuser/RobotSettings.txt"),
                 drive1Buttons(0),
                 drive2Buttons(1),
                 elevatorButtons(2),
                 dsDisplay(DSDisplay::getInstance(
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

    pidGraph.setSendInterval(5);

    displayTimer->Start();
}

Robot::~Robot() {
}

void Robot::OperatorControl() {
    robotDrive->reloadPID();
    ev->reloadPID();

    while (IsEnabled() && IsOperatorControl()) {
        if (driveStick2->GetRawButton(2)) {
            robotDrive->drive(driveStick1->GetY(), driveStick2->GetX(),
                              true);
        }
        else {
            robotDrive->drive(driveStick1->GetY(), driveStick2->GetX());
        }

        // Manual state machine
        if (elevatorButtons.releasedButton(2)) {
            ev->setManualMode(!ev->isManualMode());
        }

        // Automatic preset buttons (7-12)
        if (elevatorButtons.releasedButton(7)) {
            ev->raiseElevator(0);
        }
        if (elevatorButtons.releasedButton(8)) {
            ev->raiseElevator(1);
        }
        if (elevatorButtons.releasedButton(9)) {
            ev->raiseElevator(2);
        }
        if (elevatorButtons.releasedButton(10)) {
            ev->raiseElevator(3);
        }
        if (elevatorButtons.releasedButton(11)) {
            ev->raiseElevator(4);
        }
        if (elevatorButtons.releasedButton(12)) {
            ev->raiseElevator(5);
        }

        // Set manual value
        ev->setManualLiftSpeed(shootStick->GetY());

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

        if (drive2Buttons.releasedButton(12)) {
            ev->resetEncoder();
        }

        // Opens intake if the elevator is at the same level as it
        if (ev->getHeight() < 11 && !ev->isManualMode() &&
            ev->getIntakeGrab()) {
            ev->intakeGrab(false);
        }


        // Ensure that the intake is open if the tines are open
        if(ev->isManualMode() && ev->getElevatorGrab()) {
        	ev->intakeGrab(true);
        }

        // Poll the limit reset limit switch
        ev->pollLimitSwitch();

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

        dsDisplay.addData("EV_LEVEL_INCHES", ev->getHeight());
        dsDisplay.addData("INTAKE_ARMS_CLOSED", ev->getIntakeGrab());
        dsDisplay.addData("ARMS_CLOSED", ev->getElevatorGrab());
        dsDisplay.addData("ENCODER_LEFT", robotDrive->getLeftDist());
        dsDisplay.addData("ENCODER_RIGHT", robotDrive->getRightDist());
        std::cout << "EV_HEIGHT=" << std::left << std::setw(20) <<
            ev->getHeight()
            	/* << "EV_RAWHEIGHT=" << std::left << std::setw(20) <<
			ev->getRawHeight() */
                  << "EV_SETPOINT=" << std::left << std::setw(20) <<
            ev->getSetpoint()
                  << std::endl;

        std::string name("EL_LEVEL_");
        for (int i = 0; i < 6; i++) {
            std::string name("EL_LEVEL_");

            if (ev->getHeight() == ev->getLevel(i) && ev->onTarget()) {
                dsDisplay.addData(name + std::to_string(i), DSDisplay::active);
            }
            else if (ev->getHeight() < ev->getLevel(i + 1)) {
                dsDisplay.addData(name + std::to_string(i), DSDisplay::standby);
                dsDisplay.addData(name + std::to_string(
                                      i + 1), DSDisplay::standby);
            }
            else {
                dsDisplay.addData(name + std::to_string(i),
                                  DSDisplay::inactive);
            }
        }
        dsDisplay.sendToDS();
    }

    dsDisplay.receiveFromDS();
    insight.receiveFromDS();
}

START_ROBOT_CLASS(Robot);

