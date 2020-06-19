// Copyright (c) 2015-2020 FRC Team 3512. All Rights Reserved.

#include "Robot.hpp"

using namespace std::chrono_literals;

void Robot::AutoOneToteInit() {
    autoOneToteSM.SetState("IDLE");
    autoOneToteSM.run();

    elevator.SetManualMode(false);
}

void Robot::AutoOneTotePeriodic() {
    if (autoOneToteSM.GetState() != "IDLE") {
        autoOneToteSM.run();
        elevator.UpdateState();
    }
}

StateMachine Robot::MakeAutoOneToteSM() {
    StateMachine autoSM{"AUTO_ONE_TOTE"};

    State state{"IDLE"};
    state.transition = [this] { return "SEEK_GARBAGECAN_UP"; };
    state.exit = [this] { elevator.SetIntakeDirection(Elevator::S_STOPPED); };
    autoSM.AddState(std::move(state));

    state = State{"SEEK_GARBAGECAN_UP"};
    state.entry = [this] {
        elevator.RaiseElevator(Elevator::kGarbageCanHeight);
        elevator.StowIntake(false);
    };
    state.transition = [this] {
        if (elevator.AtGoal()) {
            return "MOVE_TO_TOTE";
        } else {
            return "";
        }
    };
    autoSM.AddState(std::move(state));

    state = State{"MOVE_TO_TOTE"};
    state.entry = [this] { autoTimer.Reset(); };
    state.transition = [this] {
        if (autoTimer.HasPeriodPassed(1_s)) {
            return "AUTOSTACK";
        } else {
            return "";
        }
    };
    state.run = [this] { drivetrain.Drive(-0.3, 0, false); };
    autoSM.AddState(std::move(state));

    state = State{"AUTOSTACK"};
    state.entry = [this] {
        autoTimer.Reset();
        elevator.IntakeGrab(true);
        elevator.SetIntakeDirection(Elevator::S_REVERSE);
    };
    state.transition = [this] {
        if (autoTimer.HasPeriodPassed(1_s)) {
            return "TURN";
        } else {
            return "";
        }
    };
    autoSM.AddState(std::move(state));

    state = State{"TURN"};
    state.entry = [this] {
        autoTimer.Reset();
        elevator.SetIntakeDirection(Elevator::S_REVERSE);
    };
    state.transition = [this] {
        if (autoTimer.HasPeriodPassed(1_s)) {
            return "RUN_AWAY";
        } else {
            return "";
        }
    };
    state.run = [this] { drivetrain.Drive(-0.3, -0.3, true); };
    state.exit = [this] { drivetrain.Drive(0.0, 0.0, false); };
    autoSM.AddState(std::move(state));

    state = State{"RUN_AWAY"};
    state.entry = [this] {
        autoTimer.Reset();
        elevator.SetIntakeDirection(Elevator::S_REVERSE);
    };
    state.transition = [this] {
        if (autoTimer.HasPeriodPassed(3_s)) {
            return "IDLE";
        } else {
            return "";
        }
    };
    state.run = [this] { drivetrain.Drive(-0.3, 0, false); };
    state.exit = [this] { drivetrain.Drive(0, 0, false); };
    autoSM.AddState(std::move(state));

    return autoSM;
}
