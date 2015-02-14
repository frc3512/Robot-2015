// =============================================================================
// File Name: AutonMotionProfile.cpp
// Description: Drives set distance with motion profiles
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "../Robot.hpp"

void Robot::AutonMotionProfile() {
    robotDrive->setLeftManual(0.f);
    robotDrive->setRightManual(0.f);
    robotDrive->setLeftSetpoint(0.f);
    robotDrive->setRightSetpoint(0.f);
    robotDrive->resetProfile();

    // Reload drive PID constants
    settings.update();
    robotDrive->setPIDSourceParameter(PIDSource::kDistance);
    robotDrive->reloadPID();

    BezierCurve curve;
    curve.push_back(std::make_pair(0.0, 0.0));
    curve.push_back(std::make_pair(0.0, 16.7));
    curve.push_back(std::make_pair(0.0, 33.3));
    curve.push_back(std::make_pair(0.0, 50.0));

    robotDrive->resetEncoders();

    autonTimer->Reset();

    // Move robot forward
    robotDrive->setGoal(curve, autonTimer->Get());
    while (IsAutonomous() && IsEnabled() && !robotDrive->atGoal()) {
        DS_PrintOut();

        robotDrive->updateSetpoint(autonTimer->Get());
        robotDrive->setLeftSetpoint(
            robotDrive->BezierTrapezoidProfile::getLeftSetpoint());
        robotDrive->setRightSetpoint(
            robotDrive->BezierTrapezoidProfile::getRightSetpoint());

        Wait(0.01);
    }

    // Stop moving
    robotDrive->setLeftManual(0.f);
    robotDrive->setRightManual(0.f);
}

