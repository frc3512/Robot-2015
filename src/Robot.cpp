#include "Robot.hpp"
#include <cmath>
#include "DriverStation.h"
#include <iostream>
#include <iomanip>

Robot::Robot() : settings("/home/lvuser/RobotSettings.txt"),
                 drive1Buttons(0),
                 drive2Buttons(1),
                 evButtons(2),
                 dsDisplay(DSDisplay::getInstance(
                               settings.getInt("DS_Port"))),
                 pidGraph(3513) {
    robotDrive = std::make_unique<DriveTrain>();
    ev = std::make_unique<Elevator>();

    driveStick1 = std::make_unique<Joystick>(0);
    driveStick2 = std::make_unique<Joystick>(1);
    shootStick = std::make_unique<Joystick>(2);
    autonTimer = std::make_unique<Timer>();
    displayTimer = std::make_unique<Timer>();

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
        if (evButtons.releasedButton(5)) {
            ev->setManualMode(!ev->isManualMode());
        }

        // Auto-stacking mode
        if (evButtons.releasedButton(3)) {
            ev->stackTotes();
        }

        // Automatic preset buttons (7-12)
        if (evButtons.releasedButton(7)) {
            ev->raiseElevator("EV_LEVEL_0");
        }
        if (evButtons.releasedButton(8)) {
            ev->raiseElevator("EV_LEVEL_1");
        }
        if (evButtons.releasedButton(9)) {
            ev->raiseElevator("EV_LEVEL_2");
        }
        if (evButtons.releasedButton(10)) {
            ev->raiseElevator("EV_LEVEL_3");
        }
        if (evButtons.releasedButton(11)) {
            ev->raiseElevator("EV_LEVEL_4");
        }
        if (evButtons.releasedButton(12)) {
            ev->raiseElevator("EV_LEVEL_5");
        }

        // Set manual value
        ev->setManualLiftSpeed(evStick->GetY());

        if (evButtons.releasedButton(1)) {
            ev->elevatorGrab(!ev->getElevatorGrab());
        }
        if (evButtons.releasedButton(2)) {
            ev->intakeGrab(!ev->getIntakeGrab());
        }
        if (evButtons.releasedButton(4)) {
            ev->stowIntake(!ev->isIntakeStowed());
        }

        if (evStick->GetPOV() == 180) {
            ev->setIntakeDirection(Elevator::S_FORWARD);
        }
        else if (evStick->GetPOV() == 0) {
            ev->setIntakeDirection(Elevator::S_REVERSED);
        }
        else {
            ev->setIntakeDirection(Elevator::S_STOPPED);
        }

        if (drive2Buttons.releasedButton(12)) {
            ev->resetEncoder();
        }

        /* Opens intake if the elevator is at the same level as it or if the
         * tines are open
         */
        if (((ev->getSetpoint() < 11 && !ev->isManualMode()) ||
             !ev->getElevatorGrab()) && ev->getIntakeGrab()) {
            ev->intakeGrab(false);
        }

        // Poll the limit reset limit switch
        ev->pollLimitSwitch();

        // Update the elevator automatic stacking state
        ev->updateState();

        drive1Buttons.updateButtons();
        drive2Buttons.updateButtons();
        evButtons.updateButtons();

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
             *  ev->getRawHeight() */
                  << "EV_SETPOINT=" << std::left << std::setw(20) <<
            ev->getSetpoint()
                  << std::endl;

        std::cout << "On target: " << ev->onTarget() << std::endl;

        std::string name("EL_LEVEL_");
        for (int i = 0; i < 6; i++) {
            std::string name("EL_LEVEL_");

            if (ev->getHeight() ==
                ev->getLevelHeight(name + std::to_string(i)) &&
                ev->onTarget()) {
                dsDisplay.addData(name + std::to_string(i), DSDisplay::active);
            }
            else if (ev->getHeight() <
                     ev->getLevelHeight(name + std::to_string(i + 1))) {
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
}

START_ROBOT_CLASS(Robot);

