// Copyright (c) 2020-2021 FRC Team 3512. All Rights Reserved.

#pragma once

#include <ctre/phoenix/motorcontrol/can/TalonSRX.h>

class CANEncoder {
public:
    CANEncoder(ctre::phoenix::motorcontrol::can::TalonSRX& motor,
               double distancePerPulse = 1.0, bool reverseDirection = false);

    double GetDistance();

    double GetRate();

    void Reset();

private:
    ctre::phoenix::motorcontrol::can::TalonSRX& m_motor;

    double m_distancePerPulse;
};
