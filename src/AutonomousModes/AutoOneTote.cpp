// =============================================================================
// File Name: AutoOneTote.cpp
// Description: Drives forward and picks up one tote
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "../Robot.hpp"
#include "../StateMachine.hpp"

void Robot::AutoOneTote() {
    State* state = new State("IDLE");
    state->endFunc = [this] { ev->setIntakeDirection(Elevator::S_STOPPED); };

    StateMachine oneToteSM(state);

    state = new State("SEEK_GARBAGECAN_UP");
    state->initFunc = [this] { ev->raiseElevator("EV_GARBAGECAN_LEVEL"); };
    state->advanceFunc = [this] {
        if (ev->atGoal()) {
            return "MOVE_TO_TOTE";
        }
        else {
            return "";
        }
    };
    oneToteSM.addState(state);
    oneToteSM.setInitialState("SEEK_GARBAGECAN_UP");

    state = new State("MOVE_TO_TOTE");
    state->initFunc = [this] {
        autoTimer->Reset();
        autoTimer->Start();
    };
    state->advanceFunc = [this] {
        if (autoTimer->HasPeriodPassed(1.0)) {
            return "AUTOSTACK";
        }
        else {
            return "";
        }
    };
    state->periodicFunc = [this] { robotDrive->drive(-0.3, 0, false); };
    oneToteSM.addState(state);

    state = new State("AUTOSTACK");
    state->initFunc = [this] {
        autoTimer->Reset();
        autoTimer->Start();
        ev->stowIntake(false);
        ev->intakeGrab(true);
        ev->setIntakeDirection(Elevator::S_REVERSED);

        // ev->stackTotes();
    };
    state->advanceFunc = [this] {
        if (autoTimer->HasPeriodPassed(1.0)) {
            return "TURN";
        }
        else {
            return "";
        }
    };
    oneToteSM.addState(state);

    state = new State("TURN");
    state->initFunc = [this] {
        autoTimer->Reset();
        autoTimer->Start();
        ev->setIntakeDirection(Elevator::S_REVERSED);
    };
    state->advanceFunc = [this] {
        if (autoTimer->HasPeriodPassed(1.4)) {
            return "RUN_AWAY";
        }
        else {
            return "";
        }
    };
    state->periodicFunc = [this] { robotDrive->drive(-0.3, -0.3, true); };
    state->endFunc = [this] { robotDrive->drive(0.0, 0.0, false); };
    oneToteSM.addState(state);

    state = new State("RUN_AWAY");
    state->initFunc = [this] {
        autoTimer->Reset();
        autoTimer->Start();
        ev->setIntakeDirection(Elevator::S_REVERSED);
    };
    state->advanceFunc = [this] {
        if (autoTimer->HasPeriodPassed(5.0)) {
            return "IDLE";
        }
        else {
            return "";
        }
    };
    state->periodicFunc = [this] { robotDrive->drive(-0.3, 0, false); };
    state->endFunc = [this] { robotDrive->drive(0, 0, false); };
    oneToteSM.addState(state);

    ev->setManualMode(false);
    oneToteSM.start();

    while (IsAutonomous() && IsEnabled()) {
        DS_PrintOut();

        oneToteSM.run();
        ev->updateState();
        // ev->intakeGrab(false);

        Wait(0.01);
    }
}

