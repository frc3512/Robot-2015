// =============================================================================
// File Name: AutoOneCanLeft.cpp
// Description: Drives forward and picks up one can
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "../Robot.hpp"
#include "../StateMachine.hpp"

void Robot::AutoOneCanLeft() {
    StateMachine autoSM("AUTO_ONE_CAN_LEFT");

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
            return "GRAB_CAN";
        }
        else {
            return "";
        }
    };
    autoSM.addState(std::move(state));

    state = std::make_unique<State>("GRAB_CAN");
    state->entry = [this] {
        autoTimer.Reset();
        ev.elevatorGrab(true);
    };
    state->transition = [this] {
        if (autoTimer.HasPeriodPassed(0.2)) {
            return "SEEK_GARBAGECAN_UP";
        }
        else {
            return "";
        }
    };
    autoSM.addState(std::move(state));

    state = std::make_unique<State>("SEEK_GARBAGECAN_UP");
    state->entry = [this] {
        ev.raiseElevator("EV_TOTE_4");
    };
    state->transition = [this] {
        if (ev.atGoal()) {
            return "DRIVE_FORWARD";
        }
        else {
            return "";
        }
    };
    autoSM.addState(std::move(state));

    state = std::make_unique<State>("DRIVE_FORWARD");
    state->entry = [this] {
        autoTimer.Reset();
    };
    state->transition = [this] {
        if (autoTimer.HasPeriodPassed(0.8)) {
            return "IDLE";
        }
        else {
            return "";
        }
    };
    state->run = [this] { robotDrive.drive(-0.3, 0, false); };
    state->exit = [this] { robotDrive.drive(0, 0, false); };
    autoSM.addState(std::move(state));

    ev.setManualMode(false);

    autoSM.run();
    while (IsAutonomous() && IsEnabled() && autoSM.getState() != "IDLE") {
        DS_PrintOut();

        autoSM.run();
        ev.updateState();

        std::this_thread::sleep_for(10ms);
    }
}

