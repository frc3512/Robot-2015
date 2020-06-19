// Copyright (c) 2015-2020 FRC Team 3512. All Rights Reserved.

#pragma once

#include <ctre/phoenix/motorcontrol/can/WPI_TalonSRX.h>

#include <frc/SpeedControllerGroup.h>
#include <frc/controller/ProfiledPIDController.h>
#include <frc/drive/DifferentialDrive.h>

#include "CANEncoder.hpp"

/**
 * Provides an interface for this year's drive train
 */
class Drivetrain {
public:
    static constexpr units::feet_per_second_t kMaxV = 80_in / 1_s;
    static constexpr units::feet_per_second_squared_t kMaxA = 80_in / 1_s / 2_s;

    Drivetrain();

    /* Drives robot with given speed and turn values [-1..1].
     * This is a convenience function for use in Operator Control.
     */
    void Drive(double throttle, double turn, bool isQuickTurn = false);

    /**
     * Sets encoder distances to 0.
     */
    void ResetEncoders();

    /**
     * Returns left encoder distance.
     */
    units::inch_t GetLeftDistance();

    /**
     * Returns right encoder distance.
     */
    units::inch_t GetRightDistance();

    void SetLeftGoal(units::foot_t goal);

    void SetRightGoal(units::foot_t goal);

    void SetLeftVoltage(units::volt_t voltage);
    void SetRightVoltage(units::volt_t voltage);

    // Returns true if controllers are at the goal
    bool LeftAtGoal() const;
    bool RightAtGoal() const;

    /**
     * Resets initial controller setpoints to current encoder measurements.
     */
    void SetSetpointsToMeasurements();

    /**
     * Runs closed-loop position control on motors.
     */
    void UpdateControllers();

private:
    ctre::phoenix::motorcontrol::can::WPI_TalonSRX m_frontLeftMotor{4};
    ctre::phoenix::motorcontrol::can::WPI_TalonSRX m_backLeftMotor{1};
    ctre::phoenix::motorcontrol::can::WPI_TalonSRX m_frontRightMotor{5};
    ctre::phoenix::motorcontrol::can::WPI_TalonSRX m_backRightMotor{8};

    CANEncoder m_leftEncoder{m_frontLeftMotor, true};
    CANEncoder m_rightEncoder{m_frontRightMotor, true};

    frc::SpeedControllerGroup m_leftGrbx{m_frontLeftMotor, m_backLeftMotor};
    frc::SpeedControllerGroup m_rightGrbx{m_frontRightMotor, m_backRightMotor};

    frc::DifferentialDrive m_drive{m_leftGrbx, m_rightGrbx};

    frc::ProfiledPIDController<units::feet> m_leftController{
        5, 0, 2, frc::TrapezoidProfile<units::feet>::Constraints{kMaxV, kMaxA},
        20_ms};
    frc::ProfiledPIDController<units::feet> m_rightController{
        8, 0, 3, frc::TrapezoidProfile<units::feet>::Constraints{kMaxV, kMaxA},
        20_ms};
};
