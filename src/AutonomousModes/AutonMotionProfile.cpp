// =============================================================================
// File Name: AutonMotionProfile.cpp
// Description: Drives set distance with motion profiles
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "../Robot.hpp"
#include "../MotionProfile/BezierCurve.hpp"

void Robot::AutonMotionProfile() {
    // Reload drive PID constants
    settings.update();
    robotDrive->reloadPID();

    BezierCurve curve;
    curve.push_back(std::make_pair(0.0, 0.0));
    curve.push_back(std::make_pair(365.0, 0.0));

    robotDrive->resetEncoders();

    Wait(0.5);

    // Move robot 365 cm forward
    robotDrive->setGoal(curve, autonTimer->Get());
    while (IsAutonomous() && IsEnabled() && !robotDrive->atGoal()) {
        DS_PrintOut();

        robotDrive->updateSetpoint(autonTimer->Get());
        robotDrive->setLeftSetpoint(robotDrive->getLeftSetpoint());
        robotDrive->setRightSetpoint(robotDrive->getRightSetpoint());

        Wait(0.01);
    }

    // Stop moving
    robotDrive->setLeftManual(0.f);
    robotDrive->setRightManual(0.f);
}

