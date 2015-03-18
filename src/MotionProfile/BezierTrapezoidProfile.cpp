// =============================================================================
// File Name: BezierTrapezoidProfile.cpp
// Description: Provides trapezoidal velocity control and follows a given Bézier
//             curve
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "BezierTrapezoidProfile.hpp"
#include <cmath>

BezierTrapezoidProfile::BezierTrapezoidProfile(double maxV, double timeToMaxV) :
    TrapezoidProfile(maxV, timeToMaxV) {
    setMaxVelocity(maxV);
    setTimeToMaxV(timeToMaxV);
    setMode(SetpointMode::distance);

    m_width = 0.0;
    m_leftSetpoint = 0.0;
    m_rightSetpoint = 0.0;
}

double BezierTrapezoidProfile::updateSetpoint(double curTime,
                                              double curSource) {
    double period = curTime - m_lastTime;

    m_varMutex.lock();

    if (m_mode == SetpointMode::distance) {
        if (curTime < m_timeToMaxVelocity) {
            // Accelerate up
            m_setpoint += (m_acceleration * curTime) * period * m_sign;
            m_leftSetpoint +=
                getLeftVelocity(curTime,
                                m_acceleration * curTime) * period * m_sign;
            m_rightSetpoint += getRightVelocity(curTime,
                                                m_acceleration * curTime) *
                               period * m_sign;
        }
        else if (curTime < m_timeFromMaxVelocity) {
            // Maintain max velocity
            m_setpoint += (m_profileMaxVelocity * period * m_sign);
            m_leftSetpoint +=
                getLeftVelocity(curTime,
                                m_profileMaxVelocity) * period * m_sign;
            m_rightSetpoint +=
                getRightVelocity(curTime,
                                 m_profileMaxVelocity) * period * m_sign;
        }
        else if (curTime < m_timeTotal) {
            // Accelerate down
            double decelTime = curTime - m_timeFromMaxVelocity;
            double v = m_profileMaxVelocity - m_acceleration * decelTime;

            m_setpoint += v * period * m_sign;
            m_leftSetpoint += getLeftVelocity(curTime, v) * period * m_sign;
            m_rightSetpoint += getRightVelocity(curTime, v) * period * m_sign;
        }
    }
    else if (m_mode == SetpointMode::velocity) {
        if (curTime < m_timeToMaxVelocity) {
            // Accelerate up
            m_setpoint = (m_acceleration * curTime) * m_sign;
            m_leftSetpoint =
                getLeftVelocity(curTime, m_acceleration * curTime) * m_sign;
            m_rightSetpoint =
                getRightVelocity(curTime, m_acceleration * curTime) * m_sign;
        }
        else if (curTime < m_timeFromMaxVelocity) {
            // Maintain max velocity
            m_setpoint = m_profileMaxVelocity * m_sign;
            m_leftSetpoint =
                getLeftVelocity(curTime, m_profileMaxVelocity) * m_sign;
            m_rightSetpoint =
                getRightVelocity(curTime, m_profileMaxVelocity) * m_sign;
        }
        else if (curTime < m_timeTotal) {
            // Accelerate down
            double decelTime = curTime - m_timeFromMaxVelocity;
            double v = m_profileMaxVelocity + (-m_acceleration * decelTime);

            m_setpoint = v * m_sign;
            m_leftSetpoint = getLeftVelocity(curTime, v) * m_sign;
            m_rightSetpoint = getRightVelocity(curTime, v) * m_sign;
        }
    }

    m_varMutex.unlock();

    m_lastTime = curTime;
    return m_setpoint;
}

double BezierTrapezoidProfile::getLeftSetpoint() const {
    return m_leftSetpoint;
}

double BezierTrapezoidProfile::getRightSetpoint() const {
    return m_rightSetpoint;
}

double BezierTrapezoidProfile::setGoal(const BezierCurve& curve, double t) {
    m_curve = curve;

    return TrapezoidProfile::setGoal(t, m_curve.getArcLength(0, 1));
}

void BezierTrapezoidProfile::resetProfile() {
    TrapezoidProfile::resetProfile();

    m_leftSetpoint = 0.0;
    m_rightSetpoint = 0.0;
}

void BezierTrapezoidProfile::setWidth(double width) {
    m_width = width;
}

double BezierTrapezoidProfile::getLeftVelocity(double t, double v) const {
    return (1.0 - m_curve.getCurvature(t / m_timeTotal) * m_width / 2.0) * v;
}

double BezierTrapezoidProfile::getRightVelocity(double t, double v) const {
    return (1.0 + m_curve.getCurvature(t / m_timeTotal) * m_width / 2.0) * v;
}

