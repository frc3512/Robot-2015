// Copyright (c) 2015-2021 FRC Team 3512. All Rights Reserved.

#include "Robot.hpp"

void Robot::AutoResetElevator() {
    elevator.SetManualMode(false);
    elevator.StowIntake(true);
    elevator.SetIntakeDirection(Elevator::S_STOPPED);

    // Seek ground
    elevator.RaiseElevator(Elevator::kGroundHeight);
    while (!elevator.AtGoal()) {
        autonChooser.YieldToMain();
        if (!IsAutonomousEnabled()) {
            return;
        }
    }
}
