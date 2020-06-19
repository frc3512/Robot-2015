// Copyright (c) 2015-2020 FRC Team 3512. All Rights Reserved.

#include "Robot.hpp"

using namespace std::chrono_literals;

void Robot::AutoOneCanRightInit() {
    autoOneCanRightSM.SetState("IDLE");
    autoOneCanRightSM.run();

    elevator.SetManualMode(false);
}

void Robot::AutoOneCanRightPeriodic() {
    if (autoOneCanRightSM.GetState() != "IDLE") {
        autoOneCanRightSM.run();
        elevator.UpdateState();
    }
}

StateMachine Robot::MakeAutoOneCanRightSM() {
    StateMachine autoSM{"AUTO_ONE_CAN_RIGHT"};

    State state{"IDLE"};
    state.transition = [this] { return "SEEK_GROUND"; };
    state.exit = [this] { elevator.SetIntakeDirection(Elevator::S_STOPPED); };
    autoSM.AddState(std::move(state));

    state = State{"SEEK_GROUND"};
    state.entry = [this] { elevator.RaiseElevator(Elevator::kGroundHeight); };
    state.transition = [this] {
        if (elevator.AtGoal()) {
            return "GRAB_CAN";
        } else {
            return "";
        }
    };
    autoSM.AddState(std::move(state));

    state = State{"GRAB_CAN"};
    state.entry = [this] {
        autoTimer.Reset();
        elevator.ElevatorGrab(true);
    };
    state.transition = [this] {
        if (autoTimer.HasPeriodPassed(0.2_s)) {
            return "SEEK_GARBAGECAN_UP";
        } else {
            return "";
        }
    };
    autoSM.AddState(std::move(state));

    state = State{"SEEK_GARBAGECAN_UP"};
    state.entry = [this] { elevator.RaiseElevator(Elevator::kToteHeight4); };
    state.transition = [this] {
        if (elevator.AtGoal()) {
            return "IDLE";
        } else {
            return "";
        }
    };
    autoSM.AddState(std::move(state));

    return autoSM;
}
