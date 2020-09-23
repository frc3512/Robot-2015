// Copyright (c) 2020 FRC Team 3512. All Rights Reserved.

#include "TalonSRXGroup.hpp"

void TalonSRXGroup::Set(double speed) {
    using namespace ctre::phoenix::motorcontrol;
    m_leader.Set(TalonSRXControlMode::PercentOutput,
                 m_isInverted ? -speed : speed);
    m_speed = speed;
}

double TalonSRXGroup::Get() const { return m_isInverted ? -m_speed : m_speed; }

void TalonSRXGroup::SetInverted(bool isInverted) { m_isInverted = isInverted; }

bool TalonSRXGroup::GetInverted() const { return m_isInverted; }

void TalonSRXGroup::Disable() {
    using namespace ctre::phoenix::motorcontrol;
    m_leader.Set(TalonSRXControlMode::PercentOutput, 0.0);
    m_speed = 0.0;
}

void TalonSRXGroup::StopMotor() {
    using namespace ctre::phoenix::motorcontrol;
    m_leader.Set(TalonSRXControlMode::PercentOutput, 0.0);
    m_speed = 0.0;
}

void TalonSRXGroup::PIDWrite(double output) { Set(output); }
