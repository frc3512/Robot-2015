// =============================================================================
// File Name: TrapezoidProfile.cpp
// Description: Provides trapezoidal velocity control
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "TrapezoidProfile.hpp"

TrapezoidProfile::TrapezoidProfile(double maxV, double timeToMaxV) :
    m_setpoint(0.0) {
    setMaxVelocity(maxV);
    setTimeToMaxV(timeToMaxV);
    setMode(SetpointMode::distance);
}

TrapezoidProfile::~TrapezoidProfile() {
}

double TrapezoidProfile::updateSetpoint(double curTime, double curSource) {
    double period = curTime - m_lastTime;

    m_varMutex.lock();

    if (m_mode == SetpointMode::distance) {
        if (curTime < m_timeToMaxVelocity) {
            // Accelerate up
            m_setpoint += (m_acceleration * curTime) * period * m_sign;
        }
        else if (curTime < m_timeFromMaxVelocity) {
            // Maintain max velocity
            m_setpoint += (m_velocity * period * m_sign);
        }
        else if (curTime < m_timeTotal) {
            // Accelerate down
            double decelTime = curTime - m_timeFromMaxVelocity;
            double v = m_velocity + (-m_acceleration * decelTime);

            m_setpoint += v * period * m_sign;
        }
    }
    else if (m_mode == SetpointMode::velocity) {
        if (curTime < m_timeToMaxVelocity) {
            // Accelerate up
            m_setpoint = (m_acceleration * curTime) * m_sign;
        }
        else if (curTime < m_timeFromMaxVelocity) {
            // Maintain max velocity
            m_setpoint = m_velocity * m_sign;
        }
        else if (curTime < m_timeTotal) {
            // Accelerate down
            double decelTime = curTime - m_timeFromMaxVelocity;
            double v = m_velocity + (-m_acceleration * decelTime);

            m_setpoint = v * m_sign;
        }
    }

    m_varMutex.unlock();

    m_lastTime = curTime;
    return m_setpoint;
}

double TrapezoidProfile::setGoal(double t, double goal, double curSource) {
    m_varMutex.lock();

    m_setpoint = goal - curSource;

    m_sign = (m_setpoint < 0) ? -1.0 : 1.0;
    m_timeToMaxVelocity = m_velocity / m_acceleration;

    double deltaPosMaxV = (m_sign * m_setpoint) -
                          (m_timeToMaxVelocity * m_velocity);
    double timeAtMaxV = deltaPosMaxV / m_velocity;

    m_timeFromMaxVelocity = m_timeToMaxVelocity + timeAtMaxV;
    m_timeTotal = m_timeFromMaxVelocity + m_timeToMaxVelocity;

    m_varMutex.unlock();

    m_lastTime = t;

    if (m_mode == SetpointMode::distance) {
        // Set setpoint to current distance since setpoint hasn't moved yet
        return curSource;
    }
    else if (m_mode == SetpointMode::velocity) {
        // Set setpoint to zero since setpoint hasn't moved yet
        return 0.f;
    }
    else {
        return curSource;
    }
}

bool TrapezoidProfile::atGoal() {
    return m_lastTime >= m_timeTotal;
}

void TrapezoidProfile::setMaxVelocity(double v) {
    m_velocity = v;
}

void TrapezoidProfile::setTimeToMaxV(double timeToMaxV) {
    m_acceleration = m_velocity / timeToMaxV;
}

void TrapezoidProfile::resetTime() {
    m_lastTime = 0.0;
}

void TrapezoidProfile::setMode(SetpointMode mode) {
    m_mode = mode;
}

SetpointMode TrapezoidProfile::getMode() {
    return m_mode;
}

