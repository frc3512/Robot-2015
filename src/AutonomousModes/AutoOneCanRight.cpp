// Copyright (c) FRC Team 3512, Spartatroniks 2015-2016. All Rights Reserved.

#include "../Robot.hpp"
#include "../StateMachine.hpp"

// Drives forward and picks up one can
void Robot::AutoOneCanRight() {
    StateMachine autoSM("AUTO_ONE_CAN_RIGHT");

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
            return "GRAB_CAN";
        } else {
            return "";
        }
    };
    autoSM.AddState(std::move(state));

    state = std::make_unique<State>("GRAB_CAN");
    state->entry = [this] {
        autoTimer.Reset();
        ev.elevatorGrab(true);
    };
    state->transition = [this] {
        if (autoTimer.HasPeriodPassed(0.2)) {
            return "SEEK_GARBAGECAN_UP";
        } else {
            return "";
        }
    };
    autoSM.AddState(std::move(state));

    state = std::make_unique<State>("SEEK_GARBAGECAN_UP");
    state->entry = [this] { ev.raiseElevator("EV_TOTE_4"); };
    state->transition = [this] {
        if (ev.atGoal()) {
            return "IDLE";
        } else {
            return "";
        }
    };
    autoSM.AddState(std::move(state));

    ev.setManualMode(false);

    autoSM.run();
    while (IsAutonomous() && IsEnabled() && autoSM.GetState() != "IDLE") {
        DS_PrintOut();

        autoSM.run();
        ev.updateState();

        std::this_thread::sleep_for(10ms);
    }
}
