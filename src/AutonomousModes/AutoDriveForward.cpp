// =============================================================================
// File Name: AutoDriveForward.cpp
// Description: Drives forward
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "../Robot.hpp"

void Robot::AutoDriveForward() {
    Timer timer;
    timer.Start();

    while (!timer.HasPeriodPassed(3.0) && IsAutonomous() && IsEnabled()) {
        robotDrive->drive(1.0, 0.0, false);
    }
    robotDrive->drive(0.0, 0.0, false);
}

