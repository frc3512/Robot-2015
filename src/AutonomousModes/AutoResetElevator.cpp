// Copyright (c) FRC Team 3512, Spartatroniks 2015-2016. All Rights Reserved.

#include "../Robot.hpp"
#include "../StateMachine.hpp"

// Seeks elevator to ground to reset its encoders
void Robot::AutoResetElevator() {
    StateMachine autoSM("AUTO_RESET_ELEVATOR");

    auto state = std::make_unique<State>("IDLE");
    state->transition = [this] { return "SEEK_GROUND"; };
    state->exit = [this] {
        ev.setIntakeDirectionLeft(Elevator::S_STOPPED);
        ev.setIntakeDirectionRight(Elevator::S_STOPPED);
    };
    autoSM.AddState(std::move(state));
    autoSM.SetState("IDLE");

    state = std::make_unique<State>("SEEK_GROUND");
    state->entry = [this] { ev.raiseElevator("EV_GROUND"); };
    state->transition = [this] {
        if (ev.atGoal()) {
            return "IDLE";
        } else {
            return "";
        }
    };
    autoSM.AddState(std::move(state));

    ev.setManualMode(false);
    ev.stowIntake(true);

    autoSM.run();
    while (IsAutonomous() && IsEnabled() && autoSM.GetState() != "IDLE") {
        DS_PrintOut();

        autoSM.run();
        ev.updateState();

        std::this_thread::sleep_for(10ms);
    }
}
