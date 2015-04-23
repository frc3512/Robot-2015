// =============================================================================
// File Name: AutoOneCan.cpp
// Description: Drives forward and picks up one can
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "../Robot.hpp"
#include "../StateMachine.hpp"

void Robot::AutoOneCan() {
    StateMachine autoSM;

    State* state = new State("IDLE");
    state->advanceFunc = [this] { return "SEEK_GROUND"; };
    state->endFunc = [this] {
        ev->setIntakeDirectionLeft(Elevator::S_STOPPED);
        ev->setIntakeDirectionRight(Elevator::S_STOPPED);
    };
    autoSM.addState(state);
    autoSM.setState("IDLE");

    state = new State("SEEK_GROUND");
    state->initFunc = [this] {
        ev->raiseElevator("EV_GROUND");
    };
    state->advanceFunc = [this] {
        if (ev->atGoal()) {
            return "OPEN_TINES_AND_DRIVE";
        }
        else {
            return "";
        }
    };
    autoSM.addState(state);

    state = new State("OPEN_TINES_AND_DRIVE");
    state->initFunc = [this] {
        autoTimer->Reset();
        ev->elevatorGrab(false);
    };
    state->advanceFunc = [this] {
        if (autoTimer->HasPeriodPassed(1.0)) {
            return "GRAB_CAN";
        }
        else {
            return "";
        }
    };
    state->periodicFunc = [this] { robotDrive->drive(-0.3, 0, false); };
    state->endFunc = [this] { robotDrive->drive(0, 0, false); };
    autoSM.addState(state);

    state = new State("GRAB_CAN");
    state->initFunc = [this] {
        autoTimer->Reset();
        ev->elevatorGrab(true);
    };
    state->advanceFunc = [this] {
        if (autoTimer->HasPeriodPassed(0.3)) {
            return "SEEK_GARBAGECAN_UP";
        }
        else {
            return "";
        }
    };
    autoSM.addState(state);

    state = new State("SEEK_GARBAGECAN_UP");
    state->initFunc = [this] {
        ev->raiseElevator("EV_TOTE_4");
    };
    state->advanceFunc = [this] {
        if (ev->atGoal()) {
            return "IDLE";
        }
        else {
            return "";
        }
    };
    autoSM.addState(state);

    ev->setManualMode(false);

    while (IsAutonomous() && IsEnabled()) {
        DS_PrintOut();

        autoSM.run();
        ev->updateState();

        std::this_thread::sleep_for(10ms);
    }
}

