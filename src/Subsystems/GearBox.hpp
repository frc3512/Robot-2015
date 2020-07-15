// Copyright (c) 2015-2020 FRC Team 3512. All Rights Reserved.

#pragma once

#include <memory>

#include <CANTalon.h>
#include <PIDOutput.h>
#include <PIDSource.h>

#include "GearBoxBase.hpp"

namespace frc {
class Encoder;
class PIDController;
}  // namespace frc

/**
 * Represents a gear box with up to 3 motors and an encoder
 */
template <class T>
class GearBox : public GearBoxBase<T>, public PIDOutput {
public:
    GearBox(int shifterChan, int encA, int encB, int motor1, int motor2 = -1,
            int motor3 = -1);

    // Enables PID controller automatically and sets its setpoint
    void setSetpoint(float setpoint);

    float getSetpoint() const;

    // Disables PID controller and sets the motor speeds manually
    void setManual(float value);

    // Returns current speed/position/voltage setting of motor controller(s)
    float get(Grbx::PIDMode mode = Grbx::Raw) const;

    // Set P, I, and D terms for PID controller
    void setPID(float p, float i, float d);

    // Set feed-forward term on PID controller
    void setF(float f);

    // Calls Encoder::SetDistancePerPulse internally
    void setDistancePerPulse(double distancePerPulse);

    // Resets encoder distance to 0
    void resetEncoder();

    // Reverses gearbox drive direction
    void setMotorReversed(bool reverse);

    // Returns motor reversal state of gearbox
    bool isMotorReversed() const;

    // Reverses gearbox drive direction
    void setEncoderReversed(bool reverse);

    // Shifts gearbox to another gear if available
    void setGear(bool gear);

    // Gets current gearbox gear if available (false if not)
    bool getGear() const;

    bool onTarget() const;

    void resetPID();

    // Determines whether encoder returns distance or rate from PIDGet()
    void setPIDSourceType(PIDSourceType pidSource);

private:
    // Sets motor speed to 'output'
    void PIDWrite(float output);

    std::unique_ptr<frc::PIDController> m_pid;
    std::shared_ptr<frc::Encoder> m_encoder;
};

#include "GearBox.inl"

template <>
class GearBox<CANTalon> : public GearBoxBase<CANTalon> {
public:
    GearBox(int shifterChan, int motor1, int motor2 = -1, int motor3 = -1);

    // Enables PID controller automatically and sets its setpoint
    void setSetpoint(float setpoint);

    float getSetpoint() const;

    // Disables PID controller and sets the motor speeds manually
    void setManual(float value);

    // Returns current speed/position/voltage setting of motor controller(s)
    float get(Grbx::PIDMode mode = Grbx::Raw) const;

    // Set P, I, and D terms for PID controller
    void setPID(float p, float i, float d);

    // Set feed-forward term on PID controller
    void setF(float f);

    void setDistancePerPulse(double distancePerPulse);

    // Resets encoder distance to 0
    void resetEncoder();

    // Reverses gearbox drive direction
    void setEncoderReversed(bool reverse);

    bool onTarget() const;

    void resetPID();

    // Determines whether encoder returns distance or rate from PIDGet()
    void setControlMode(
        CANTalon::ControlMode ctrlMode = CANTalon::kPercentVbus);

    // Set soft limits of PID controller
    void setSoftPositionLimits(double forwardLimit, double backwardLimit);

    bool isFwdLimitSwitchClosed() const;
    bool isRevLimitSwitchClosed() const;

    void setIZone(unsigned int value);

    void setCloseLoopRampRate(double value);
    void setProfile(bool secondProfile);
};

#include "GearBoxCANTalon.inl"
