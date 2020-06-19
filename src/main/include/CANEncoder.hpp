// Copyright (c) 2020 FRC Team 3512. All Rights Reserved.

#pragma once

#include <ctre/phoenix/motorcontrol/can/TalonSRX.h>

class CANEncoder {
public:
    CANEncoder(ctre::phoenix::motorcontrol::can::TalonSRX& motor,
               bool reverseDirection = false);

    void SetDistancePerPulse(double distancePerPulse);

    double GetDistance();

    double GetRate();

    void Reset();

private:
    ctre::phoenix::motorcontrol::can::TalonSRX& m_motor;

    double m_distancePerPulse;
};
