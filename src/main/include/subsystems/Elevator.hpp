// Copyright (c) 2015-2021 FRC Team 3512. All Rights Reserved.

#pragma once

#include <atomic>
#include <map>
#include <string>
#include <thread>
#include <vector>

#include <ctre/phoenix/motorcontrol/can/WPI_TalonSRX.h>
#include <frc/Solenoid.h>
#include <frc/controller/ProfiledPIDController.h>
#include <frc2/Timer.h>
#include <units/acceleration.h>
#include <units/length.h>
#include <units/velocity.h>
#include <units/voltage.h>

#include "CANDigitalInput.hpp"
#include "CANEncoder.hpp"
#include "StateMachine.hpp"
#include "TalonSRXGroup.hpp"

/**
 * Provides an interface for the robot's elevator
 */
class Elevator {
public:
    enum IntakeMotorState {
        S_STOPPED,
        S_FORWARD,
        S_REVERSE,
        S_ROTATE_CCW,
        S_ROTATE_CW
    };

    static constexpr units::inch_t kGroundHeight = 0.0_in;
    static constexpr units::inch_t kToteHeight1 = 16.0_in;
    static constexpr units::inch_t kToteHeight2 = 28.76_in;
    static constexpr units::inch_t kToteHeight3 = 42.14_in;
    static constexpr units::inch_t kToteHeight4 = 54.32_in;
    static constexpr units::inch_t kToteHeight5 = 67.68_in;
    static constexpr units::inch_t kStepHeight = 6.25_in;
    static constexpr units::inch_t kHalfToteOffset = 4.0_in;
    static constexpr units::inch_t kGarbageCanHeight = 28.76_in;
    static constexpr units::inch_t kAutoDropHeight = 5.0_in;
    static constexpr units::inch_t kMaxHeight = 70.0_in;
    static constexpr units::feet_per_second_t kMaxVUp = 88_in / 1_s;
    static constexpr units::feet_per_second_squared_t kMaxAUp =
        88_in / 1_s / 0.4_s;
    static constexpr units::feet_per_second_t kMaxVDown = 91.26_in / 1_s;
    static constexpr units::feet_per_second_squared_t kMaxADown =
        91.26_in / 1_s / 0.4_s;
    static constexpr units::feet_per_second_t kMaxVDownZeroing = 35.63_in / 1_s;

    Elevator();

    // Actuates elevator tines in/out
    void ElevatorGrab(bool state);
    bool IsElevatorGrabbed() const;

    // Actuates intake arms in/out
    void IntakeGrab(bool state);
    bool IsIntakeGrabbed() const;

    // Stows/unstows intake arms
    void StowIntake(bool state);
    bool IsIntakeStowed() const;

    // Actuates container graba in/out
    void ContainerGrab(bool state);
    bool IsContainerGrabbed() const;

    // Sets direction of intake wheels
    void SetIntakeDirection(IntakeMotorState state);
    IntakeMotorState GetIntakeDirection() const;

    // Sets speed of lift gearbox directly if manual input is enabled
    void SetManualLiftSpeed(units::volt_t value);
    units::meters_per_second_t GetManualLiftSpeed();

    void SetManualMode(bool on);
    bool IsManualMode() const;

    // Sets setpoint for elevator PID controller
    void SetHeight(units::meter_t height);
    units::meter_t GetHeight();

    // Returns if controller is at goal
    bool AtGoal() const;

    void ResetEncoders();

    // Takes a string representing the name of the height
    void RaiseElevator(units::meter_t level);

    void StackTotes();
    bool IsStacking() const;
    void CancelStack();

    // Periodically update the tote auto stacking state
    void UpdateState();

private:
    frc::Solenoid m_elevatorGrabber{3};
    frc::Solenoid m_containerGrabber{4};

    ctre::phoenix::motorcontrol::can::WPI_TalonSRX m_liftLeftMotor{7};
    ctre::phoenix::motorcontrol::can::WPI_TalonSRX m_liftRightMotor{2};
    TalonSRXGroup m_liftGrbx{m_liftLeftMotor, m_liftRightMotor};
    CANEncoder m_liftEncoder{m_liftLeftMotor, true};
    bool m_manual = false;

    // Intake
    IntakeMotorState m_intakeState = S_STOPPED;
    frc::Solenoid m_intakeStower{1};
    frc::Solenoid m_intakeGrabber{2};
    ctre::phoenix::motorcontrol::can::WPI_TalonSRX m_intakeLeftMotor{3};
    ctre::phoenix::motorcontrol::can::WPI_TalonSRX m_intakeRightMotor{6};

    frc::ProfiledPIDController<units::inches> m_controller{
        3.0, 0.0, 0.0, {kMaxVUp, kMaxAUp}};
    CANDigitalInput m_limitSwitch{m_liftLeftMotor};
    bool m_lastLimitSwitchValue = false;

    StateMachine m_autoStackSM{"AUTO_STACK"};
    frc2::Timer m_grabTimer;
    bool m_startAutoStacking = false;

    /**
     * Set the goal for the elevator height motion profile.
     */
    void SetGoal(units::meter_t height);
};
