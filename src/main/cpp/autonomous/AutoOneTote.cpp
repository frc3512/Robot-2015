// Copyright (c) 2015-2021 FRC Team 3512. All Rights Reserved.

#include "Robot.hpp"

void Robot::AutoOneTote() {
    elevator.SetManualMode(false);
    elevator.SetIntakeDirection(Elevator::S_STOPPED);

    // Seek garbage can up
    elevator.RaiseElevator(Elevator::kGarbageCanHeight);
    elevator.StowIntake(false);
    while (!elevator.AtGoal()) {
        autonChooser.YieldToMain();
        if (!IsAutonomousEnabled()) {
            return;
        }
    }

    // Move to tote
    frc2::Timer timer;
    timer.Start();
    while (!timer.HasPeriodPassed(1_s)) {
        drivetrain.Drive(-0.3, 0, false);

        autonChooser.YieldToMain();
        if (!IsAutonomousEnabled()) {
            return;
        }
    }

    // Autostack
    timer.Reset();
    elevator.IntakeGrab(true);
    elevator.SetIntakeDirection(Elevator::S_REVERSE);
    while (!timer.HasPeriodPassed(1_s)) {
        autonChooser.YieldToMain();
        if (!IsAutonomousEnabled()) {
            return;
        }
    }

    // Turn
    timer.Reset();
    elevator.SetIntakeDirection(Elevator::S_REVERSE);
    while (!timer.HasPeriodPassed(1_s)) {
        drivetrain.Drive(-0.3, -0.3, true);

        autonChooser.YieldToMain();
        if (!IsAutonomousEnabled()) {
            return;
        }
    }
    drivetrain.Drive(0.0, 0.0, false);

    // Run away
    timer.Reset();
    elevator.SetIntakeDirection(Elevator::S_REVERSE);
    while (!timer.HasPeriodPassed(3_s)) {
        drivetrain.Drive(-0.3, 0, false);

        autonChooser.YieldToMain();
        if (!IsAutonomousEnabled()) {
            return;
        }
    }
    drivetrain.Drive(0, 0, false);
}
