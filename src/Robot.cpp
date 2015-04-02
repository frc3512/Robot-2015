// =============================================================================
// File Name: Robot.cpp
// Description: Implements the main robot class
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "Robot.hpp"
#include <cmath>
#include <iostream>

Robot::Robot() : settings("/home/lvuser/RobotSettings.txt"),
                 drive1Buttons(0),
                 drive2Buttons(1),
                 evButtons(2),
                 dsDisplay(DSDisplay::getInstance(settings.getInt("DS_Port"))),
                 pidGraph(3513) {
    robotDrive = std::make_unique<DriveTrain>();
    ev = std::make_unique<Elevator>();

    driveStick1 = std::make_unique<Joystick>(0);
    driveStick2 = std::make_unique<Joystick>(1);
    evStick = std::make_unique<Joystick>(2);
    autoTimer = std::make_unique<Timer>();
    displayTimer = std::make_unique<Timer>();
    accumTimer = std::make_unique<Timer>();

    dsDisplay.addAutoMethod("Noop Auton", &Robot::AutoNoop, this);
    dsDisplay.addAutoMethod("DriveForward", &Robot::AutoDriveForward, this);
    dsDisplay.addAutoMethod("OneTote", &Robot::AutoOneTote, this);

    pidGraph.setSendInterval(5ms);

    displayTimer->Start();
}

void Robot::OperatorControl() {
    while (IsEnabled() && IsOperatorControl()) {
        if (driveStick2->GetRawButton(2)) {
            robotDrive->drive(driveStick1->GetY(), driveStick2->GetX(), true);
        }
        else {
            robotDrive->drive(driveStick1->GetY(), driveStick2->GetX());
        }

        // Open/close tines
        if (evButtons.releasedButton(1)) {
            ev->elevatorGrab(!ev->isElevatorGrabbed());
        }

        // Open/close intake
        if (evButtons.releasedButton(2)) {
            ev->intakeGrab(!ev->isIntakeGrabbed());
        }

        // Start auto-stacking mode
        if (evButtons.releasedButton(3)) {
            ev->stackTotes();
        }

        // Manual height control
        if (evButtons.releasedButton(4)) {
            ev->setManualMode(!ev->isManualMode());
        }

        // Stow intake
        if (evButtons.releasedButton(5)) {
            ev->stowIntake(!ev->isIntakeStowed());
        }

        // Open/close container grabber
        if (evButtons.releasedButton(6)) {
            ev->containerGrab(!ev->isContainerGrabbed());
        }

        // Automatic preset buttons (7-12)
        // TODO: Special case for level 0
        if (evButtons.releasedButton(8)) {
            ev->raiseElevator("EV_GROUND");
        }
        if (evButtons.releasedButton(7)) {
            ev->raiseElevator("EV_TOTE_1");
        }
        if (evButtons.releasedButton(10)) {
            ev->raiseElevator("EV_TOTE_2");
        }
        if (evButtons.releasedButton(9)) {
            ev->raiseElevator("EV_TOTE_3");
        }
        if (evButtons.releasedButton(12)) {
            ev->raiseElevator("EV_TOTE_4");
        }
        if (evButtons.releasedButton(11)) {
            ev->raiseElevator("EV_TOTE_5");
        }

        // Set manual value
        ev->setManualLiftSpeed(evStick->GetY());

        // Controls intake left side
        if (driveStick1->GetPOV() == 0 || evStick->GetPOV() == 0) {
            ev->setIntakeDirectionLeft(Elevator::S_FORWARD);
        }
        else if (driveStick1->GetPOV() == 90 || evStick->GetPOV() == 90) {
            ev->setIntakeDirectionLeft(Elevator::S_ROTATE_CCW);
        }
        else if (driveStick1->GetPOV() == 180 || evStick->GetPOV() == 180 ||
                 driveStick1->GetRawButton(1)) {
            ev->setIntakeDirectionLeft(Elevator::S_REVERSE);
        }
        else if (driveStick1->GetPOV() == 270 || evStick->GetPOV() == 270) {
            ev->setIntakeDirectionLeft(Elevator::S_ROTATE_CW);
        }
        else {
            ev->setIntakeDirectionLeft(Elevator::S_STOPPED);
        }

        // Controls intake right side
        if (driveStick2->GetPOV() == 0 || evStick->GetPOV() == 0) {
            ev->setIntakeDirectionRight(Elevator::S_FORWARD);
        }
        else if (driveStick2->GetPOV() == 90 || evStick->GetPOV() == 90) {
            ev->setIntakeDirectionRight(Elevator::S_ROTATE_CCW);
        }
        else if (driveStick2->GetPOV() == 180 || evStick->GetPOV() == 180 ||
                 driveStick2->GetRawButton(1)) {
            ev->setIntakeDirectionRight(Elevator::S_REVERSE);
        }
        else if (driveStick2->GetPOV() == 270 || evStick->GetPOV() == 270) {
            ev->setIntakeDirectionRight(Elevator::S_ROTATE_CW);
        }
        else {
            ev->setIntakeDirectionRight(Elevator::S_STOPPED);
        }

        // Accumulate assisted automatic mode
        double deltaT = accumTimer->Get();
        accumTimer->Reset();
        accumTimer->Start();

        double evStickY = evStick->GetY();
        evStickY = 0;
        manualAverage.addValue(evStickY * ev->getMaxVelocity() * deltaT);

        // Deadband
        if (applyDeadband(manualAverage.get(), 0.05) &&
            applyDeadband(evStickY, 0.05)) {
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
        if (ev->isIntakeGrabbed()) {
            if ((ev->getSetpoint() < 11 && !ev->isManualMode()) ||
                !ev->isElevatorGrabbed() ||
                ev->isIntakeStowed()) {
                ev->intakeGrab(false);
            }
        }

        // Poll the limit reset limit switch
        ev->pollLiftLimitSwitches();

        // Update the elevator automatic stacking state
        ev->updateState();

        drive1Buttons.updateButtons();
        drive2Buttons.updateButtons();
        evButtons.updateButtons();

        DS_PrintOut();

        std::this_thread::sleep_for(10ms);
    }
}

void Robot::Autonomous() {
    autoTimer->Reset();
    autoTimer->Start();

    robotDrive->resetEncoders();
    dsDisplay.execAutonomous();
}

void Robot::Disabled() {
    while (IsDisabled()) {
        DS_PrintOut();
        std::this_thread::sleep_for(100ms);
    }

    robotDrive->reloadPID();
    ev->reloadPID();
}

void Robot::DS_PrintOut() {
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

        dsDisplay.addData("ENCODER_LEFT", robotDrive->getLeftDist());
        dsDisplay.addData("ENCODER_RIGHT", robotDrive->getRightDist());
        dsDisplay.addData("EV_POS_DISP", ev->getHeight());
        dsDisplay.addData("EV_POS", 100 * ev->getHeight() / 60);

        dsDisplay.addData("ARMS_CLOSED", ev->isElevatorGrabbed());
        dsDisplay.addData("INTAKE_ARMS_CLOSED", ev->isIntakeGrabbed());
        dsDisplay.addData("CONTAINER_GRABBER_CLOSED", ev->isContainerGrabbed());

        dsDisplay.sendToDS();
    }

    dsDisplay.receiveFromDS();
}

float Robot::applyDeadband(float value, float deadband) {
    if (fabs(value) > deadband) {
        if (value > 0) {
            return (value - deadband) / (1 - deadband);
        }
        else {
            return (value + deadband) / (1 - deadband);
        }
    }
    else {
        return 0.f;
    }
}

START_ROBOT_CLASS(Robot);

