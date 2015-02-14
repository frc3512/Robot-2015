// =============================================================================
// File Name: DriveTrain.cpp
// Description: Provides an interface for this year's drive train
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "DriveTrain.hpp"

#include <cmath>
#include <CANTalon.h>

#define max(x, y) (((x) > (y)) ? (x) : (y))

#ifndef M_PI
#define M_PI 3.14159265
#endif

const float DriveTrain::maxWheelSpeed = 274.f;

DriveTrain::DriveTrain() : BezierTrapezoidProfile(maxWheelSpeed, 3.f),
                           m_settings("RobotSettings.txt") {
    m_settings.update();

    m_deadband = 0.02f;
    m_sensitivity = m_settings.getFloat("LOW_GEAR_SENSITIVE");
    // TODO Does robot start in low gear?

    m_oldTurn = 0.f;
    m_quickStopAccumulator = 0.f;
    m_negInertiaAccumulator = 0.f;

#if 0
    // For WPILib PID loop
    m_leftFrontGrbx = new GearBox<CANTalon>(-1, -1, -1, 4);
    m_leftBackGrbx = new GearBox<CANTalon>(-1, -1, -1, 1);
    m_rightFrontGrbx = new GearBox<CANTalon>(-1, -1, -1, 5);
    m_rightBackGrbx = new GearBox<CANTalon>(-1, -1, -1, 8);
#else
    // For CANTalon PID loop
    m_leftFrontGrbx = new GearBox<CANTalon>(-1, false, 4);
    m_leftBackGrbx = new GearBox<CANTalon>(-1, false, 1);
    m_rightFrontGrbx = new GearBox<CANTalon>(-1, false, 5);
    m_rightBackGrbx = new GearBox<CANTalon>(-1, false, 8);
#endif

    m_leftFrontGrbx->setMotorReversed(true);
    m_leftBackGrbx->setMotorReversed(true);

    m_isDefencive = (false);
    // c = PI * 10.16cm [wheel diameter]
    // dPerP = c / pulses
    m_leftFrontGrbx->setDistancePerPulse(
        ((3.14159265 * 10.16) / 360.0) * 1.0 / 3.0);
    m_rightFrontGrbx->setDistancePerPulse(
        ((3.14159265 * 10.16) / 360.0) * 1.0 / 3.0);
    m_leftBackGrbx->setDistancePerPulse(
        ((3.14159265 * 10.16) / 360.0) * 1.0 / 3.0);
    m_rightBackGrbx->setDistancePerPulse(
        ((3.14159265 * 10.16) / 360.0) * 1.0 / 3.0);

    reloadPID();
}

DriveTrain::~DriveTrain() {
    delete m_leftFrontGrbx;
    delete m_rightFrontGrbx;
    delete m_leftBackGrbx;
    delete m_rightBackGrbx;
}

void DriveTrain::drive(float throttle, float turn, bool isQuickTurn) {
    // Modified Cheesy Drive; base code courtesy of FRC Team 254

    throttle *= -1;

    if (m_isDefencive == true) {
        throttle *= -1;
        turn *= -1;
    }
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

    float turnNonLinearity = m_settings.getFloat("TURN_NON_LINEARITY");

    /* Apply a sine function that's scaled to make turning sensitivity feel better.
     * turnNonLinearity should never be zero, but can be close
     */
    turn = sin(M_PI / 2.0 * turnNonLinearity * turn) /
           sin(M_PI / 2.0 * turnNonLinearity);

    double angularPower = 0.f;
    double linearPower = throttle;
    double leftPwm = linearPower, rightPwm = linearPower;

    // Negative inertia!
    double negInertiaScalar;
    if (getGear()) {
        negInertiaScalar = m_settings.getFloat("INERTIA_HIGH_GEAR");
    }
    else {
        if (turn * negInertia > 0) {
            negInertiaScalar = m_settings.getFloat("INERTIA_LOW_DAMPEN");
        }
        else {
            if (fabs(turn) > 0.65) {
                negInertiaScalar =
                    m_settings.getFloat("INERTIA_LOW_HIGH_TURN");
            }
            else {
                negInertiaScalar =
                    m_settings.getFloat("INERTIA_LOW_LOW_TURN");
            }
        }
    }

    m_negInertiaAccumulator += negInertia * negInertiaScalar; // adds negInertiaPower

    // Apply negative inertia
    turn += m_negInertiaAccumulator;
    if (m_negInertiaAccumulator > 1) {
        m_negInertiaAccumulator -= 1;
    }
    else if (m_negInertiaAccumulator < -1) {
        m_negInertiaAccumulator += 1;
    }
    else {
        m_negInertiaAccumulator = 0;
    }

    // QuickTurn!
    if (isQuickTurn) {
        if (fabs(linearPower) < 0.2) {
            double alpha = 0.1;
            m_quickStopAccumulator = (1 - alpha) * m_quickStopAccumulator +
                                     alpha * limit(turn, 1.f) * 5;
        }

        angularPower = turn;
    }
    else {
        angularPower = fabs(throttle) * turn * m_sensitivity -
                       m_quickStopAccumulator;

        if (m_quickStopAccumulator > 1) {
            m_quickStopAccumulator -= 1;
        }
        else if (m_quickStopAccumulator < -1) {
            m_quickStopAccumulator += 1;
        }
        else {
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
    }
    else if (rightPwm > 1.0) {
        // If overpowered turning enabled
        if (isQuickTurn) {
            leftPwm -= (rightPwm - 1.f);
        }

        rightPwm = 1.0;
    }
    else if (leftPwm < -1.0) {
        // If overpowered turning enabled
        if (isQuickTurn) {
            rightPwm += (-leftPwm - 1.f);
        }

        leftPwm = -1.0;
    }
    else if (rightPwm < -1.0) {
        // If overpowered turning enabled
        if (isQuickTurn) {
            leftPwm += (-rightPwm - 1.f);
        }

        rightPwm = -1.0;
    }
    m_leftFrontGrbx->setManual(leftPwm);
    m_rightFrontGrbx->setManual(rightPwm);
    if (true /*!isQuickTurn */) {
        m_leftBackGrbx->setManual(leftPwm);
        m_rightBackGrbx->setManual(rightPwm);
    }
    else {
        m_leftBackGrbx->setManual(0.0);
        m_rightBackGrbx->setManual(0.0);
    }
}

void DriveTrain::setDeadband(float band) {
    m_deadband = band;
}

void DriveTrain::resetEncoders() {
    m_leftFrontGrbx->resetEncoder();
    m_rightFrontGrbx->resetEncoder();
    m_leftBackGrbx->resetEncoder();
    m_rightBackGrbx->resetEncoder();
}

void DriveTrain::reloadPID() {
    m_settings.update();

    float p = 0.f;
    float i = 0.f;
    float d = 0.f;

    p = m_settings.getFloat("PID_DRIVE_P");
    i = m_settings.getFloat("PID_DRIVE_I");
    d = m_settings.getFloat("PID_DRIVE_D");

    m_leftFrontGrbx->setPID(p, i, d);
    m_rightBackGrbx->setPID(p, i, d);
    m_leftFrontGrbx->setPID(p, i, d);
    m_rightBackGrbx->setPID(p, i, d);
}

void DriveTrain::setLeftSetpoint(double setpt) {
    m_leftFrontGrbx->setSetpoint(setpt);
    m_leftBackGrbx->setSetpoint(setpt);
}

void DriveTrain::setRightSetpoint(double setpt) {
    m_rightFrontGrbx->setSetpoint(setpt);
    m_rightBackGrbx->setSetpoint(setpt);
}

void DriveTrain::setLeftManual(float value) {
    m_leftFrontGrbx->setManual(value);
    m_leftBackGrbx->setManual(value);
}

void DriveTrain::setRightManual(float value) {
    m_rightFrontGrbx->setManual(value);
    m_rightBackGrbx->setManual(value);
}

double DriveTrain::getLeftDist() {
    return (m_leftFrontGrbx->get(Grbx::Position) +
            m_leftBackGrbx->get(Grbx::Position)) / 2.f;
}

double DriveTrain::getRightDist() {
    return (m_rightFrontGrbx->get(Grbx::Position) +
            m_rightBackGrbx->get(Grbx::Position)) / 2.f;
}

double DriveTrain::getLeftRate() {
    return (m_leftFrontGrbx->get(Grbx::Speed) +
            m_leftBackGrbx->get(Grbx::Speed)) / 2.f;
}

double DriveTrain::getRightRate() {
    return (m_rightFrontGrbx->get(Grbx::Speed) +
            m_rightBackGrbx->get(Grbx::Speed)) / 2.f;
}

double DriveTrain::getLeftSetpoint() {
    return m_leftFrontGrbx->getSetpoint();
}

double DriveTrain::getRightSetpoint() {
    return m_rightFrontGrbx->getSetpoint();
}

void DriveTrain::setControlMode(CANTalon::ControlMode ctrlMode) {
    m_leftFrontGrbx->setControlMode(ctrlMode);
    m_rightBackGrbx->setControlMode(ctrlMode);
    m_leftFrontGrbx->setControlMode(ctrlMode);
    m_rightBackGrbx->setControlMode(ctrlMode);
}

void DriveTrain::setGear(bool gear) {
    m_leftFrontGrbx->setGear(gear);
    m_rightBackGrbx->setGear(gear);
    m_leftFrontGrbx->setGear(gear);
    m_rightBackGrbx->setGear(gear);


    /* Update turning sensitivity
     * Lower value makes robot turn less when full turn is commanded.
     * Value of 1 (default) makes robot's turn radius the smallest.
     * Value of 0 makes robot unable to turn unless QuickTurn is enabled.
     */

    // If high gear
    if (gear) {
        m_sensitivity = m_settings.getFloat("HIGH_GEAR_SENSITIVE");
    }
    else {
        m_sensitivity = m_settings.getFloat("LOW_GEAR_SENSITIVE");
    }
}

bool DriveTrain::getGear() const {
    return m_leftFrontGrbx->getGear();
}
void DriveTrain::setDefencive(bool defencive) {
    m_isDefencive = defencive;
}
// returns true if drive train is reversed
bool DriveTrain::getDefencive() {
    if (m_isDefencive == true) {
        return true;
    }
    else {
        return false;
    }
}

float DriveTrain::applyDeadband(float value) {
    if (fabs(value) > m_deadband) {
        if (value > 0) {
            return (value - m_deadband) / (1 - m_deadband);
        }
        else {
            return (value + m_deadband) / (1 - m_deadband);
        }
    }
    else {
        return 0.f;
    }
}

