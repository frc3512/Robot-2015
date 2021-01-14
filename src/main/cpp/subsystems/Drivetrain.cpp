// Copyright (c) 2015-2021 FRC Team 3512. All Rights Reserved.

#include "subsystems/Drivetrain.hpp"

#include <cmath>

#include <ctre/phoenix/motorcontrol/can/TalonSRX.h>

Drivetrain::Drivetrain() { m_leftGrbx.SetInverted(true); }

void Drivetrain::Drive(double throttle, double turn, bool isQuickTurn) {
    m_drive.CurvatureDrive(throttle, turn, isQuickTurn);
}

void Drivetrain::ResetEncoders() {
    m_leftEncoder.Reset();
    m_rightEncoder.Reset();
}

units::inch_t Drivetrain::GetLeftDistance() {
    return units::inch_t{m_leftEncoder.GetDistance()};
}

units::inch_t Drivetrain::GetRightDistance() {
    return units::inch_t{m_rightEncoder.GetDistance()};
}

void Drivetrain::SetLeftGoal(units::foot_t goal) {
    m_leftController.SetGoal(goal);
}

void Drivetrain::SetRightGoal(units::foot_t goal) {
    m_rightController.SetGoal(goal);
}

void Drivetrain::SetLeftVoltage(units::volt_t voltage) {
    m_leftGrbx.SetVoltage(voltage);
}

void Drivetrain::SetRightVoltage(units::volt_t voltage) {
    m_rightGrbx.SetVoltage(voltage);
}

bool Drivetrain::LeftAtGoal() const { return m_leftController.AtGoal(); }

bool Drivetrain::RightAtGoal() const { return m_rightController.AtGoal(); }

void Drivetrain::SetSetpointsToMeasurements() {
    m_leftController.Reset(units::inch_t{m_leftEncoder.GetDistance()});
    m_rightController.Reset(units::inch_t{m_rightEncoder.GetDistance()});
}

void Drivetrain::UpdateControllers() {
    m_leftGrbx.Set(
        m_leftController.Calculate(units::inch_t{m_leftEncoder.GetDistance()}));
    m_rightGrbx.Set(m_rightController.Calculate(
        units::inch_t{m_rightEncoder.GetDistance()}));
}
