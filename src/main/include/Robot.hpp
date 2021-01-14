// Copyright (c) 2015-2021 FRC Team 3512. All Rights Reserved.

#pragma once

#include <frc/Joystick.h>
#include <frc/TimedRobot.h>

#include "AutonomousChooser.hpp"
#include "subsystems/Drivetrain.hpp"
#include "subsystems/Elevator.hpp"

/**
 * Implements the main robot class
 */
class Robot : public frc::TimedRobot {
public:
    Drivetrain drivetrain;
    Elevator elevator;

    Robot();
    void TeleopPeriodic() override;
    void AutonomousInit() override;
    void AutonomousPeriodic() override;

    // Drives forward
    void AutoDriveForward();

    // Seeks elevator to ground to reset its encoders
    void AutoResetElevator();

    // Drives forward and picks up one can
    void AutoOneCanCenter();

    // Drives forward and picks up one can
    void AutoOneCanLeft();

    // Drives forward and picks up one can
    void AutoOneCanRight();

    // Drives forward and picks up one tote
    void AutoOneTote();

private:
    frc::Joystick driveStick1{0};
    frc::Joystick driveStick2{1};
    frc::Joystick appendageStick{2};

    frc3512::AutonomousChooser autonChooser{"No-op", [] {}};
};
