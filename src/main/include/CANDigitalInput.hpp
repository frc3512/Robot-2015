// Copyright (c) 2020 FRC Team 3512. All Rights Reserved.

#include <ctre/phoenix/motorcontrol/SensorCollection.h>
#include <ctre/phoenix/motorcontrol/can/TalonSRX.h>

#pragma once

class CANDigitalInput {
public:
    explicit CANDigitalInput(ctre::phoenix::motorcontrol::can::TalonSRX& motor);

    bool Get();

private:
    ctre::phoenix::motorcontrol::can::TalonSRX& m_motor;
    ctre::phoenix::motorcontrol::SensorCollection m_sensor{m_motor};
};
