// Copyright (c) 2015-2021 FRC Team 3512. All Rights Reserved.

#include <frc2/Timer.h>

#include "Robot.hpp"

void Robot::AutoDriveForward() {
    frc2::Timer timer;
    timer.Start();

    while (timer.Get() < 2.5_s) {
        drivetrain.Drive(-0.4, 0.0, false);
        autonChooser.YieldToMain();
        if (!IsAutonomousEnabled()) {
            return;
        }
    }

    drivetrain.Drive(0.0, 0.0, false);
}
