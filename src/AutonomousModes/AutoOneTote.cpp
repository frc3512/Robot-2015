// Copyright (c) 2015-2017 FRC Team 3512. All Rights Reserved.

#include "../Robot.hpp"
#include "../StateMachine.hpp"

using namespace std::chrono_literals;

// Drives forward and picks up one tote
void Robot::AutoOneTote() {
    StateMachine autoSM("AUTO_ONE_TOTE");

    auto state = std::make_unique<State>("IDLE");
    state->transition = [this] { return "SEEK_GARBAGECAN_UP"; };
    state->exit = [this] {
        ev.setIntakeDirectionLeft(Elevator::S_STOPPED);
        ev.setIntakeDirectionRight(Elevator::S_STOPPED);
    };
    autoSM.AddState(std::move(state));
    autoSM.SetState("IDLE");

    state = std::make_unique<State>("SEEK_GARBAGECAN_UP");
    state->entry = [this] {
        ev.raiseElevator("EV_GARBAGECAN_LEVEL");
        ev.stowIntake(false);
    };
    state->transition = [this] {
        if (ev.atGoal()) {
            return "MOVE_TO_TOTE";
        } else {
            return "";
        }
    };
    autoSM.AddState(std::move(state));

    state = std::make_unique<State>("MOVE_TO_TOTE");
    state->entry = [this] { autoTimer.Reset(); };
    state->transition = [this] {
        if (autoTimer.HasPeriodPassed(1.0)) {
            return "AUTOSTACK";
        } else {
            return "";
        }
    };
    state->run = [this] { robotDrive.Drive(-0.3, 0, false); };
    autoSM.AddState(std::move(state));

    state = std::make_unique<State>("AUTOSTACK");
    state->entry = [this] {
        autoTimer.Reset();
        ev.intakeGrab(true);
        ev.setIntakeDirectionLeft(Elevator::S_REVERSE);
        ev.setIntakeDirectionRight(Elevator::S_REVERSE);

        // ev.stackTotes();
    };
    state->transition = [this] {
        if (autoTimer.HasPeriodPassed(1.0)) {
            return "TURN";
        } else {
            return "";
        }
    };
    autoSM.AddState(std::move(state));

    state = std::make_unique<State>("TURN");
    state->entry = [this] {
        autoTimer.Reset();
        ev.setIntakeDirectionLeft(Elevator::S_REVERSE);
        ev.setIntakeDirectionRight(Elevator::S_REVERSE);
    };
    state->transition = [this] {
        if (autoTimer.HasPeriodPassed(1.0)) {
            return "RUN_AWAY";
        } else {
            return "";
        }
    };
    state->run = [this] { robotDrive.Drive(-0.3, -0.3, true); };
    state->exit = [this] { robotDrive.Drive(0.0, 0.0, false); };
    autoSM.AddState(std::move(state));

    state = std::make_unique<State>("RUN_AWAY");
    state->entry = [this] {
        autoTimer.Reset();
        ev.setIntakeDirectionLeft(Elevator::S_REVERSE);
        ev.setIntakeDirectionRight(Elevator::S_REVERSE);
    };
    state->transition = [this] {
        if (autoTimer.HasPeriodPassed(3.0)) {
            return "IDLE";
        } else {
            return "";
        }
    };
    state->run = [this] { robotDrive.Drive(-0.3, 0, false); };
    state->exit = [this] { robotDrive.Drive(0, 0, false); };
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
