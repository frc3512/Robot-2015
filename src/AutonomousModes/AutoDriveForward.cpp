// Copyright (c) FRC Team 3512, Spartatroniks 2015-2017. All Rights Reserved.

#include <chrono>

#include "../Robot.hpp"

using namespace std::chrono_literals;

// Drives forward
void Robot::AutoDriveForward() {
    Timer timer;
    timer.Start();

    while (!timer.HasPeriodPassed(2.5) && IsAutonomous() && IsEnabled()) {
        DS_PrintOut();
        robotDrive.Drive(-0.4, 0.0, false);
        std::this_thread::sleep_for(10ms);
    }
    robotDrive.Drive(0.0, 0.0, false);
}
