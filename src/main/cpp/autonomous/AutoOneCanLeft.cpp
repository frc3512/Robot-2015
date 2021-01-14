// Copyright (c) 2015-2021 FRC Team 3512. All Rights Reserved.

#include <frc2/Timer.h>

#include "Robot.hpp"

void Robot::AutoOneCanLeft() {
    elevator.SetManualMode(false);
    elevator.SetIntakeDirection(Elevator::S_STOPPED);

    // Seek ground
    elevator.RaiseElevator(Elevator::kGroundHeight);
    while (!elevator.AtGoal()) {
        autonChooser.YieldToMain();
        if (!IsAutonomousEnabled()) {
            return;
        }
    }

    // Grab can
    frc2::Timer timer;
    timer.Start();
    elevator.ElevatorGrab(true);
    while (!timer.HasPeriodPassed(0.2_s)) {
        autonChooser.YieldToMain();
        if (!IsAutonomousEnabled()) {
            return;
        }
    }

    // Seek garbage can up
    elevator.RaiseElevator(Elevator::kToteHeight4);
    while (!elevator.AtGoal()) {
        autonChooser.YieldToMain();
        if (!IsAutonomousEnabled()) {
            return;
        }
    }

    // Drive forward
    timer.Reset();
    while (!timer.HasPeriodPassed(0.8_s)) {
        drivetrain.Drive(-0.3, 0, false);

        autonChooser.YieldToMain();
        if (!IsAutonomousEnabled()) {
            return;
        }
    }

    drivetrain.Drive(0, 0, false);
}
