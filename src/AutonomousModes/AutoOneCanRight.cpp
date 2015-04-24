// =============================================================================
// File Name: AutoOneCanRight.cpp
// Description: Drives forward and picks up one can
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "../Robot.hpp"
#include "../StateMachine.hpp"

void Robot::AutoOneCanRight() {
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
            return "GRAB_CAN";
        }
        else {
            return "";
        }
    };
    autoSM.addState(state);

    state = new State("GRAB_CAN");
    state->initFunc = [this] {
        autoTimer->Reset();
        ev->elevatorGrab(true);
    };
    state->advanceFunc = [this] {
        if (autoTimer->HasPeriodPassed(0.2)) {
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

    autoSM.run();
    while (IsAutonomous() && IsEnabled() && autoSM.getState() != "IDLE") {
        DS_PrintOut();

        autoSM.run();
        ev->updateState();

        std::this_thread::sleep_for(10ms);
    }
}

