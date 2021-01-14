// Copyright (c) 2020-2021 FRC Team 3512. All Rights Reserved.

#include "CANEncoder.hpp"

#include <ctre/phoenix/motorcontrol/FeedbackDevice.h>

CANEncoder::CANEncoder(ctre::phoenix::motorcontrol::can::TalonSRX& motor,
                       double distancePerPulse, bool reverseDirection)
    : m_motor{motor}, m_distancePerPulse{distancePerPulse} {
    motor.ConfigSelectedFeedbackSensor(
        ctre::phoenix::motorcontrol::FeedbackDevice::QuadEncoder, 0, 0);
    motor.SetSensorPhase(reverseDirection);
}

double CANEncoder::GetDistance() {
    return m_motor.GetSensorCollection().GetQuadraturePosition() *
           m_distancePerPulse;
}

double CANEncoder::GetRate() {
    return m_motor.GetSensorCollection().GetQuadratureVelocity() *
           m_distancePerPulse;
}

void CANEncoder::Reset() {
    m_motor.GetSensorCollection().SetQuadraturePosition(0);
}
