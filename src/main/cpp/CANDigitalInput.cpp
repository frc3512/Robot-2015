// Copyright (c) 2020 FRC Team 3512. All Rights Reserved.

#include "CANDigitalInput.hpp"

CANDigitalInput::CANDigitalInput(
    ctre::phoenix::motorcontrol::can::TalonSRX& motor)
    : m_motor(motor) {}

bool CANDigitalInput::Get() { return m_sensor.IsRevLimitSwitchClosed(); }
