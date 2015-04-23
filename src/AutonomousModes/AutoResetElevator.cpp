// =============================================================================
// File Name: AutoResetElevator.cpp
// Description: Seeks elevator to ground to reset its encoders
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "../Robot.hpp"
#include "../StateMachine.hpp"

void Robot::AutoResetElevator() {
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
            return "IDLE";
        }
        else {
            return "";
        }
    };
    autoSM.addState(state);

    ev->setManualMode(false);
    ev->stowIntake(true);

    while (IsAutonomous() && IsEnabled()) {
        DS_PrintOut();

        autoSM.run();
        ev->updateState();

        std::this_thread::sleep_for(10ms);
    }
}

