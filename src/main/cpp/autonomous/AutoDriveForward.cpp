// Copyright (c) 2015-2020 FRC Team 3512. All Rights Reserved.

#include <chrono>

#include "Robot.hpp"

using namespace std::chrono_literals;

static frc2::Timer timer;

void Robot::AutoDriveForwardInit() { timer.Start(); }

void Robot::AutoDriveForwardPeriodic() {
    if (timer.Get() < 2.5_s) {
        drivetrain.Drive(-0.4, 0.0, false);
    } else {
        drivetrain.Drive(0.0, 0.0, false);
    }
}
