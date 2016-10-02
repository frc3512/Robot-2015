// Copyright (c) FRC Team 3512, Spartatroniks 2015-2016. All Rights Reserved.

#include "DriveTrain.hpp"

#include <cmath>

#include <CANTalon.h>

const float DriveTrain::maxWheelSpeed = 80.0;

DriveTrain::DriveTrain() : BezierTrapezoidProfile(maxWheelSpeed, 2) {
    m_sensitivity = m_settings.GetDouble("LOW_GEAR_SENSITIVE");

    m_leftGrbx.setMotorReversed(true);
    m_leftGrbx.setEncoderReversed(true);

    m_rightGrbx.setEncoderReversed(true);

    m_leftGrbx.setDistancePerPulse(72.0 / 2800.0);
    m_rightGrbx.setDistancePerPulse(72.0 / 2800.0);

    m_leftGrbx.setManual(0.0);
    m_rightGrbx.setManual(0.0);

    setWidth(27.0);

    reloadPID();
}

void DriveTrain::Drive(float throttle, float turn, bool isQuickTurn) {
    // Modified Cheesy Drive; base code courtesy of FRC Team 254

    throttle *= -1;

    // Limit values to [-1 .. 1]
    throttle = limit(throttle, 1.f);
    turn = limit(turn, 1.f);

    /* Apply joystick deadband
     * (Negate turn since joystick X-axis is reversed)
     */
    throttle = applyDeadband(throttle);
    turn = applyDeadband(turn);

    double negInertia = turn - m_oldTurn;
    m_oldTurn = turn;

    float turnNonLinearity = m_settings.GetDouble("TURN_NON_LINEARITY");

    /* Apply a sine function that's scaled to make turning sensitivity feel
     * better. turnNonLinearity should never be zero, but can be close
     */
    turn = std::sin(M_PI / 2.0 * turnNonLinearity * turn) /
           std::sin(M_PI / 2.0 * turnNonLinearity);

    double angularPower = 0.f;
    double linearPower = throttle;
    double leftPwm = linearPower, rightPwm = linearPower;

    // Negative inertia!
    double negInertiaScalar;
    if (turn * negInertia > 0) {
        negInertiaScalar = m_settings.GetDouble("INERTIA_DAMPEN");
    } else {
        if (std::fabs(turn) > 0.65) {
            negInertiaScalar = m_settings.GetDouble("INERTIA_HIGH_TURN");
        } else {
            negInertiaScalar = m_settings.GetDouble("INERTIA_LOW_TURN");
        }
    }

    m_negInertiaAccumulator +=
        negInertia * negInertiaScalar;  // adds negInertiaPower

    // Apply negative inertia
    turn += m_negInertiaAccumulator;
    if (m_negInertiaAccumulator > 1) {
        m_negInertiaAccumulator -= 1;
    } else if (m_negInertiaAccumulator < -1) {
        m_negInertiaAccumulator += 1;
    } else {
        m_negInertiaAccumulator = 0;
    }

    // QuickTurn!
    if (isQuickTurn) {
        if (std::fabs(linearPower) < 0.2) {
            double alpha = 0.1;
            m_quickStopAccumulator = (1 - alpha) * m_quickStopAccumulator +
                                     alpha * limit(turn, 1.f) * 5;
        }

        angularPower = turn;
    } else {
        angularPower =
            std::fabs(throttle) * turn * m_sensitivity - m_quickStopAccumulator;

        if (m_quickStopAccumulator > 1) {
            m_quickStopAccumulator -= 1;
        } else if (m_quickStopAccumulator < -1) {
            m_quickStopAccumulator += 1;
        } else {
            m_quickStopAccumulator = 0.0;
        }
    }

    // Adjust straight path for turn
    leftPwm += angularPower;
    rightPwm -= angularPower;

    // Limit PWM bounds to [-1..1]
    if (leftPwm > 1.0) {
        // If overpowered turning enabled
        if (isQuickTurn) {
            rightPwm -= (leftPwm - 1.f);
        }

        leftPwm = 1.0;
    } else if (rightPwm > 1.0) {
        // If overpowered turning enabled
        if (isQuickTurn) {
            leftPwm -= (rightPwm - 1.f);
        }

        rightPwm = 1.0;
    } else if (leftPwm < -1.0) {
        // If overpowered turning enabled
        if (isQuickTurn) {
            rightPwm += (-leftPwm - 1.f);
        }

        leftPwm = -1.0;
    } else if (rightPwm < -1.0) {
        // If overpowered turning enabled
        if (isQuickTurn) {
            leftPwm += (-rightPwm - 1.f);
        }

        rightPwm = -1.0;
    }
    m_leftGrbx.setManual(leftPwm);
    m_rightGrbx.setManual(rightPwm);
}

void DriveTrain::setDeadband(float band) { m_deadband = band; }

void DriveTrain::reloadPID() {
    m_settings.Update();

    float p = 0.f;
    float i = 0.f;
    float d = 0.f;

    p = m_settings.GetDouble("PID_DRIVE_LEFT_P");
    i = m_settings.GetDouble("PID_DRIVE_LEFT_I");
    d = m_settings.GetDouble("PID_DRIVE_LEFT_D");
    m_leftGrbx.setPID(p, i, d);

    p = m_settings.GetDouble("PID_DRIVE_RIGHT_P");
    i = m_settings.GetDouble("PID_DRIVE_RIGHT_I");
    d = m_settings.GetDouble("PID_DRIVE_RIGHT_D");
    m_rightGrbx.setPID(p, i, d);
}

void DriveTrain::resetEncoders() {
    m_leftGrbx.resetEncoder();
    m_rightGrbx.resetEncoder();
}

void DriveTrain::setLeftSetpoint(double setpt) {
    m_leftGrbx.setSetpoint(setpt);
}

void DriveTrain::setRightSetpoint(double setpt) {
    m_rightGrbx.setSetpoint(setpt);
}

void DriveTrain::setLeftManual(float value) { m_leftGrbx.setManual(value); }

void DriveTrain::setRightManual(float value) { m_rightGrbx.setManual(value); }

double DriveTrain::getLeftDist() const {
    return m_leftGrbx.get(Grbx::Position);
}

double DriveTrain::getRightDist() const {
    return m_rightGrbx.get(Grbx::Position);
}

double DriveTrain::getLeftRate() const { return m_leftGrbx.get(Grbx::Speed); }

double DriveTrain::getRightRate() const { return m_rightGrbx.get(Grbx::Speed); }

double DriveTrain::getLeftSetpoint() const { return m_leftGrbx.getSetpoint(); }

double DriveTrain::getRightSetpoint() const {
    return m_rightGrbx.getSetpoint();
}

void DriveTrain::setControlMode(CANTalon::ControlMode ctrlMode) {
    m_leftGrbx.setControlMode(ctrlMode);
    m_rightGrbx.setControlMode(ctrlMode);
}

float DriveTrain::applyDeadband(float value) {
    if (std::fabs(value) > m_deadband) {
        if (value > 0) {
            return (value - m_deadband) / (1 - m_deadband);
        } else {
            return (value + m_deadband) / (1 - m_deadband);
        }
    } else {
        return 0.f;
    }
}
