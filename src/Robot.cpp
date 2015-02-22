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
    evStick = std::make_unique<Joystick>(2);
    autoTimer = std::make_unique<Timer>();
    displayTimer = std::make_unique<Timer>();
    accumTimer = std::make_unique<Timer>();

    dsDisplay.addAutoMethod("DriveForward", &Robot::AutoDriveForward, this);
    dsDisplay.addAutoMethod("OneTote", &Robot::AutoOneTote, this);
    dsDisplay.addAutoMethod("Noop Auton", &Robot::AutoNoop, this);

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
        if (evButtons.releasedButton(4)) {
            ev->stackTotes();
        }

        std::string offsetString;
        if (evStick->GetThrottle() > 0) {
            offsetString = "+EV_HALF_TOTE_OFFSET";
        }
        else {
            offsetString = "";
        }

        // Automatic preset buttons (7-12)
        // TODO: Special case for level 0
        if (evButtons.releasedButton(7)) {
            ev->raiseElevator("EV_TOTE_0" + offsetString);
        }
        if (evButtons.releasedButton(8)) {
            ev->raiseElevator("EV_TOTE_1" + offsetString);
        }
        if (evButtons.releasedButton(9)) {
            ev->raiseElevator("EV_TOTE_2" + offsetString);
        }
        if (evButtons.releasedButton(10)) {
            ev->raiseElevator("EV_TOTE_3" + offsetString);
        }
        if (evButtons.releasedButton(11)) {
            ev->raiseElevator("EV_TOTE_4" + offsetString);
        }
        if (evButtons.releasedButton(12)) {
            ev->raiseElevator("EV_TOTE_5" + offsetString);
        }

        // Set manual value
        ev->setManualLiftSpeed(evStick->GetY());

        if (evButtons.releasedButton(1)) {
            ev->elevatorGrab(!ev->isElevatorGrabbed());
        }
        if (evButtons.releasedButton(2)) {
            ev->intakeGrab(!ev->isIntakeGrabbed());
        }
        if (evButtons.releasedButton(3)) {
            ev->stowIntake(!ev->isIntakeStowed());
        }

        if (evStick->GetPOV() == 0) {
            ev->setIntakeDirection(Elevator::S_FORWARD);
        }
        else if (evStick->GetPOV() == 180) {
            ev->setIntakeDirection(Elevator::S_REVERSED);
        }
        else {
            ev->setIntakeDirection(Elevator::S_STOPPED);
        }

        if (drive2Buttons.releasedButton(12)) {
            ev->resetEncoder();
        }

        // Accumulate assised automatic mode
        double deltaT = accumTimer->Get();
        accumTimer->Reset();
        accumTimer->Start();

        double evStickY = evStick->GetY();
        evStickY = 0;
        manualAverage.addValue(evStickY * ev->getMaxVelocity() * deltaT);

        // Deadband
        if (fabs(manualAverage.get()) > 0.05 && fabs(evStickY) > 0.05) {
            if (ev->getSetpoint() + manualAverage.get() > 0
                && ev->getSetpoint() + manualAverage.get() <
                settings.getDouble("EV_MAX_HEIGHT")) {
                std::cout << "manualChangeSetpoint("
                          << manualAverage.get()
                          << ")"
                          << std::endl;
                ev->manualChangeSetpoint(manualAverage.get());
            }
        }

        /* Opens intake if the elevator is at the same level as it or if the
         * tines are open
         */
        if (((ev->getSetpoint() < 11 && !ev->isManualMode()) ||
             !ev->isElevatorGrabbed()) && ev->isIntakeGrabbed()) {
            ev->intakeGrab(false);
        }

        if (ev->isIntakeStowed() && ev->isIntakeGrabbed()) {
            ev->intakeGrab(false);
        }

        // Poll the limit reset limit switch
        ev->pollLimitSwitches();

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
    autoTimer->Reset();
    autoTimer->Start();

    dsDisplay.execAutonomous();

    autoTimer->Stop();
}

void Robot::Disabled() {
    while (IsDisabled()) {
        DS_PrintOut();
        Wait(0.1);
    }
}

void Robot::DS_PrintOut() {
    std::cout << "left=" << robotDrive->getLeftRate() << std::endl;
    std::cout << "right=" << robotDrive->getRightRate() << std::endl;

    if (pidGraph.hasIntervalPassed()) {
        pidGraph.graphData(ev->getHeight(), "Distance (EV)");
        pidGraph.graphData(ev->getSetpoint(), "Setpoint (EV)");
        pidGraph.graphData(robotDrive->getLeftDist(), "Left PV (DR)");
        pidGraph.graphData(robotDrive->getLeftSetpoint(), "Left SP (DR)");
        pidGraph.graphData(robotDrive->getRightDist(), "Right PV (DR)");
        pidGraph.graphData(robotDrive->getRightSetpoint(), "Right SP (DR)");

        pidGraph.resetInterval();
    }

    if (displayTimer->HasPeriodPassed(0.5)) {
        // Send things to DS display
        dsDisplay.clear();

        dsDisplay.addData("EV_LEVEL_INCHES", ev->getHeight());
        dsDisplay.addData("INTAKE_ARMS_CLOSED", ev->isIntakeGrabbed());
        dsDisplay.addData("ARMS_CLOSED", ev->isElevatorGrabbed());
        dsDisplay.addData("ENCODER_LEFT", robotDrive->getLeftDist());
        dsDisplay.addData("ENCODER_RIGHT", robotDrive->getRightDist());

        std::string name("EL_LEVEL_");
        for (int i = 0; i < 6; i++) {
            std::string name("EL_LEVEL_");

            if (ev->getHeight() ==
                ev->getLevelHeight(name + std::to_string(i)) &&
                ev->atGoal()) {
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

