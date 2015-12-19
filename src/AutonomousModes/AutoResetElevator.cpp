// =============================================================================
// File Name: AutoResetElevator.cpp
// Description: Seeks elevator to ground to reset its encoders
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "../Robot.hpp"
#include "../StateMachine.hpp"

void Robot::AutoResetElevator() {
    StateMachine autoSM("AUTO_RESET_ELEVATOR");

    auto state = std::make_unique<State>("IDLE");
    state->transition = [this] { return "SEEK_GROUND"; };
    state->exit = [this] {
        ev.setIntakeDirectionLeft(Elevator::S_STOPPED);
        ev.setIntakeDirectionRight(Elevator::S_STOPPED);
    };
    autoSM.addState(std::move(state));
    autoSM.setState("IDLE");

    state = std::make_unique<State>("SEEK_GROUND");
    state->entry = [this] {
        ev.raiseElevator("EV_GROUND");
    };
    state->transition = [this] {
        if (ev.atGoal()) {
            return "IDLE";
        }
        else {
            return "";
        }
    };
    autoSM.addState(std::move(state));

    ev.setManualMode(false);
    ev.stowIntake(true);

    autoSM.run();
    while (IsAutonomous() && IsEnabled() && autoSM.getState() != "IDLE") {
        DS_PrintOut();

        autoSM.run();
        ev.updateState();

        std::this_thread::sleep_for(10ms);
    }
}

