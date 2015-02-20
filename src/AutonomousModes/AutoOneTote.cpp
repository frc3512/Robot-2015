// =============================================================================
// File Name: AutoOneTote.cpp
// Description: Drives forward and picks up one tote
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "../Robot.hpp"
#include "../StateMachine.hpp"

void Robot::AutoOneTote() {
    StateMachine oneToteSM;
    State* state = new State("IDLE");
    state->endFunc = [this] { ev->setIntakeDirection(Elevator::S_STOPPED); };

    oneToteSM.addState(state);

    state = new State("SEEK_GARBAGECAN_UP");
    state->initFunc = [this] { ev->raiseElevator("EV_GARBAGECAN_LEVEL"); };
    state->advanceFunc = [this] { return ev->atGoal(); };
    oneToteSM.addState(state);

    state = new State("MOVE_TO_TOTE");
    state->initFunc = [this] {
        autoTimer->Reset();
        autoTimer->Start();
    };
    state->advanceFunc = [this] { return autoTimer->HasPeriodPassed(1.0); };
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
    state->advanceFunc = [this] { return autoTimer->HasPeriodPassed(1.0); };
    oneToteSM.addState(state);

    state = new State("TURN");
    state->initFunc = [this] {
        autoTimer->Reset();
        autoTimer->Start();
        ev->setIntakeDirection(Elevator::S_REVERSED);
    };
    state->advanceFunc = [this] { return autoTimer->HasPeriodPassed(1.4); };
    state->periodicFunc = [this] { robotDrive->drive(-0.3, -0.3, true); };
    state->endFunc = [this] { robotDrive->drive(0.0, 0.0, false); };
    oneToteSM.addState(state);

    state = new State("RUN_AWAY");
    state->initFunc = [this] {
        autoTimer->Reset();
        autoTimer->Start();
        ev->setIntakeDirection(Elevator::S_REVERSED);
    };
    state->advanceFunc = [this] { return autoTimer->HasPeriodPassed(5.0); };
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

