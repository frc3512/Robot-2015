// Copyright (c) 2015-2020 FRC Team 3512. All Rights Reserved.

#pragma once

#include <frc/Joystick.h>
#include <frc/TimedRobot.h>
#include <frc2/Timer.h>

#include "autonselector/AutonSelector.hpp"
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
    void AutoDriveForwardInit();
    void AutoDriveForwardPeriodic();

    // Seeks elevator to ground to reset its encoders
    void AutoResetElevatorInit();
    void AutoResetElevatorPeriodic();
    StateMachine MakeAutoResetElevatorSM();

    // Drives forward and picks up one can
    void AutoOneCanCenterInit();
    void AutoOneCanCenterPeriodic();
    StateMachine MakeAutoOneCanCenterSM();

    // Drives forward and picks up one can
    void AutoOneCanLeftInit();
    void AutoOneCanLeftPeriodic();
    StateMachine MakeAutoOneCanLeftSM();

    // Drives forward and picks up one can
    void AutoOneCanRightInit();
    void AutoOneCanRightPeriodic();
    StateMachine MakeAutoOneCanRightSM();

    // Drives forward and picks up one tote
    void AutoOneToteInit();
    void AutoOneTotePeriodic();
    StateMachine MakeAutoOneToteSM();

    // Drives forward
    void AutoMotionProfileInit();
    void AutoMotionProfilePeriodic();

private:
    StateMachine autoOneCanCenterSM = MakeAutoOneCanCenterSM();
    StateMachine autoOneCanLeftSM = MakeAutoOneCanLeftSM();
    StateMachine autoOneCanRightSM = MakeAutoOneCanRightSM();
    StateMachine autoOneToteSM = MakeAutoOneToteSM();
    StateMachine autoResetElevatorSM = MakeAutoResetElevatorSM();

    frc::Joystick driveStick1{0};
    frc::Joystick driveStick2{1};
    frc::Joystick appendageStick{2};

    frc2::Timer autoTimer;

    // Used for sending data to the Driver Station
    frc3512::AutonSelector autonSelector{1130};
};
