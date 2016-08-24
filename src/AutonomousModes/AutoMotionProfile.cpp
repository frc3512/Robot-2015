// Copyright (c) FRC Team 3512, Spartatroniks 2015-2016. All Rights Reserved.

#include "../Robot.hpp"

// Drives forward
void Robot::AutoMotionProfile() {
    robotDrive.setLeftManual(0.f);
    robotDrive.setRightManual(0.f);
    robotDrive.setLeftSetpoint(0.f);
    robotDrive.setRightSetpoint(0.f);
    robotDrive.resetProfile();

    robotDrive.setControlMode(CANTalon::kPosition);

    BezierCurve curve;
    curve.AddPoint(0.0, 0.0);
    curve.AddPoint(0.0, 50.0);
    curve.AddPoint(0.0, 100.0);
    curve.AddPoint(0.0, 150.0);

    robotDrive.resetEncoders();

    autoTimer.Reset();

    // Move robot forward
    robotDrive.setCurveGoal(curve, autoTimer.Get());
    while (IsAutonomous() && IsEnabled() && !robotDrive.atGoal()) {
        DS_PrintOut();

        robotDrive.updateSetpoint(autoTimer.Get());
        robotDrive.setLeftSetpoint(
            robotDrive.BezierTrapezoidProfile::getLeftSetpoint());
        robotDrive.setRightSetpoint(
            robotDrive.BezierTrapezoidProfile::getRightSetpoint());

        std::this_thread::sleep_for(10ms);
    }

    // Stop moving
    robotDrive.setLeftManual(0.f);
    robotDrive.setRightManual(0.f);

    while (IsAutonomous() && IsEnabled()) {
        DS_PrintOut();
        std::this_thread::sleep_for(10ms);
    }
}
