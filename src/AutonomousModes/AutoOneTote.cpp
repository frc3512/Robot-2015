// =============================================================================
// File Name: AutoOneTote.cpp
// Description: Drives forward and picks up one tote
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "../Robot.hpp"
#include "../StateMachine.hpp"

void Robot::AutoOneTote() {
    StateMachine autoSM;

    State* state = new State("IDLE");
    state->advanceFunc = [this] { return "SEEK_GARBAGECAN_UP"; };
    state->endFunc = [this] {
        ev.setIntakeDirectionLeft(Elevator::S_STOPPED);
        ev.setIntakeDirectionRight(Elevator::S_STOPPED);
    };
    autoSM.addState(state);
    autoSM.setState("IDLE");

    state = new State("SEEK_GARBAGECAN_UP");
    state->initFunc = [this] {
        ev.raiseElevator("EV_GARBAGECAN_LEVEL");
        ev.stowIntake(false);
    };
    state->advanceFunc = [this] {
        if (ev.atGoal()) {
            return "MOVE_TO_TOTE";
        }
        else {
            return "";
        }
    };
    autoSM.addState(state);

    state = new State("MOVE_TO_TOTE");
    state->initFunc = [this] {
        autoTimer.Reset();
    };
    state->advanceFunc = [this] {
        if (autoTimer.HasPeriodPassed(1.0)) {
            return "AUTOSTACK";
        }
        else {
            return "";
        }
    };
    state->periodicFunc = [this] { robotDrive.drive(-0.3, 0, false); };
    autoSM.addState(state);

    state = new State("AUTOSTACK");
    state->initFunc = [this] {
        autoTimer.Reset();
        ev.intakeGrab(true);
        ev.setIntakeDirectionLeft(Elevator::S_REVERSE);
        ev.setIntakeDirectionRight(Elevator::S_REVERSE);

        // ev.stackTotes();
    };
    state->advanceFunc = [this] {
        if (autoTimer.HasPeriodPassed(1.0)) {
            return "TURN";
        }
        else {
            return "";
        }
    };
    autoSM.addState(state);

    state = new State("TURN");
    state->initFunc = [this] {
        autoTimer.Reset();
        ev.setIntakeDirectionLeft(Elevator::S_REVERSE);
        ev.setIntakeDirectionRight(Elevator::S_REVERSE);
    };
    state->advanceFunc = [this] {
        if (autoTimer.HasPeriodPassed(1.0)) {
            return "RUN_AWAY";
        }
        else {
            return "";
        }
    };
    state->periodicFunc = [this] { robotDrive.drive(-0.3, -0.3, true); };
    state->endFunc = [this] { robotDrive.drive(0.0, 0.0, false); };
    autoSM.addState(state);

    state = new State("RUN_AWAY");
    state->initFunc = [this] {
        autoTimer.Reset();
        ev.setIntakeDirectionLeft(Elevator::S_REVERSE);
        ev.setIntakeDirectionRight(Elevator::S_REVERSE);
    };
    state->advanceFunc = [this] {
        if (autoTimer.HasPeriodPassed(3.0)) {
            return "IDLE";
        }
        else {
            return "";
        }
    };
    state->periodicFunc = [this] { robotDrive.drive(-0.3, 0, false); };
    state->endFunc = [this] { robotDrive.drive(0, 0, false); };
    autoSM.addState(state);

    ev.setManualMode(false);

    autoSM.run();
    while (IsAutonomous() && IsEnabled() && autoSM.getState() != "IDLE") {
        DS_PrintOut();

        autoSM.run();
        ev.updateState();

        std::this_thread::sleep_for(10ms);
    }
}

