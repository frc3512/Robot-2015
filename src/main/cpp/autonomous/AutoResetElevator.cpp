// Copyright (c) 2015-2020 FRC Team 3512. All Rights Reserved.

#include "Robot.hpp"

using namespace std::chrono_literals;

void Robot::AutoResetElevatorInit() {
    autoResetElevatorSM.SetState("IDLE");
    autoResetElevatorSM.run();

    elevator.SetManualMode(false);
    elevator.StowIntake(true);
}

void Robot::AutoResetElevatorPeriodic() {
    if (autoResetElevatorSM.GetState() != "IDLE") {
        autoResetElevatorSM.run();
        elevator.UpdateState();
    }
}

StateMachine Robot::MakeAutoResetElevatorSM() {
    StateMachine autoSM{"AUTO_RESET_ELEVATOR"};

    State state{"IDLE"};
    state.transition = [this] { return "SEEK_GROUND"; };
    state.exit = [this] { elevator.SetIntakeDirection(Elevator::S_STOPPED); };
    autoSM.AddState(std::move(state));

    state = State{"SEEK_GROUND"};
    state.entry = [this] { elevator.RaiseElevator(Elevator::kGroundHeight); };
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
