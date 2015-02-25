// =============================================================================
// File Name: SCurveProfile.cpp
// Description: Provides trapezoidal acceleration control
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "SCurveProfile.hpp"
#include <cmath>

SCurveProfile::SCurveProfile(double maxV, double maxA, double timeToMaxA) {
    setMaxVelocity(maxV);
    setTimeToMaxA(timeToMaxA);
}

SCurveProfile::~SCurveProfile() {
}

double SCurveProfile::updateSetpoint(double curTime, double curSource) {
    std::lock_guard<decltype(m_varMutex)> lock(m_varMutex);

    double tmpSP = 0.0;

    if (curTime < m_timeToMaxA) {
        // Ramp up acceleration
        tmpSP = (0.5 * m_jerk * pow(curTime, 2)) * m_sign;
    }
    else if (curTime < m_t2) {
        // Increase speed at max acceleration
        tmpSP = (0.5 * m_jerk * pow(m_timeToMaxA, 2) + m_acceleration *
                 (curTime - m_timeToMaxA)) * m_sign;
    }
    else if (curTime < m_t3) {
        // Ramp down acceleration
        tmpSP = (0.5 * m_jerk * pow(m_timeToMaxA, 2) + m_acceleration *
                 (m_t2 - m_timeToMaxA) - 0.5 * m_jerk *
                 pow(m_t2 - curTime, 2)) *
                m_sign;
    }
    else if (curTime < m_t4) {
        // Maintain max velocity
        tmpSP = m_profileMaxVelocity * m_sign;
    }
    else if (curTime < m_t5) {
        // Ramp down acceleration
        tmpSP = m_profileMaxVelocity - 0.5 * m_jerk * pow(curTime - m_t4, 2) *
                m_sign;
    }
    else if (curTime < m_t6) {
        // Decrease speed at max acceleration
        tmpSP = m_jerk * pow(m_timeToMaxA, 2) + m_acceleration *
                (m_t2 - m_timeToMaxA + m_t5 - curTime) * m_sign;
    }
    else if (curTime < m_t7) {
        // Ramp down acceleration
        tmpSP = 0.5 * pow(m_t6 - curTime, 2) * m_sign;
    }

    if (m_mode == SetpointMode::distance) {
        m_setpoint += tmpSP * (curTime - m_lastTime);
    }
    else if (m_mode == SetpointMode::velocity) {
        m_setpoint = tmpSP;
    }

    m_lastTime = curTime;
    return m_setpoint;
}

double SCurveProfile::setGoal(double t, double goal, double curSource) {
    std::lock_guard<decltype(m_varMutex)> lock(m_varMutex);

    m_goal = goal;
    m_setpoint = goal - curSource;

    m_sign = (m_setpoint < 0) ? -1.0 : 1.0;

    // If profile can't accelerate up to max velocity before decelerating
    bool shortProfile = 2.0 / 3.0 * m_acceleration * pow(m_timeToMaxA, 2) +
                        pow(m_maxVelocity, 2) / m_acceleration + m_timeToMaxA *
                        m_maxVelocity > m_sign * m_setpoint;

    if (shortProfile) {
        m_profileMaxVelocity = m_acceleration * (sqrt(m_sign * m_setpoint /
                                                      m_acceleration - 0.75 *
                                                      pow(m_timeToMaxA,
                                                          2)) - 0.5 *
                                                 m_timeToMaxA);
    }
    else {
        m_profileMaxVelocity = m_maxVelocity;
    }

    // Find times at critical points
    m_t2 = m_profileMaxVelocity / m_acceleration;
    m_t3 = m_t2 + m_timeToMaxA;
    if (shortProfile) {
        m_t4 = m_t3;
    }
    else {
        m_t4 = m_sign * m_setpoint / m_profileMaxVelocity - 2.0 / 3.0 *
               pow(m_timeToMaxA, 2) / m_t2 + 2 * m_t2 - m_timeToMaxA;
    }
    m_t5 = m_t4 + m_timeToMaxA;
    m_t6 = m_t4 + m_t2;
    m_t7 = m_t6 + m_timeToMaxA;
    m_timeTotal = m_t7;

    m_lastTime = t;

    if (m_mode == SetpointMode::distance) {
        // Set setpoint to current distance since setpoint hasn't moved yet
        m_setpoint = curSource;
        return curSource;
    }
    else if (m_mode == SetpointMode::velocity) {
        // Set setpoint to zero since setpoint hasn't moved yet
        m_setpoint = 0;
        return 0.f;
    }
    else {
        m_setpoint = 0;
        return curSource;
    }
}

void SCurveProfile::setMaxVelocity(double v) {
    m_maxVelocity = v;
}

double SCurveProfile::getMaxVelocity() const {
    return m_maxVelocity;
}

void SCurveProfile::setMaxAcceleration(double a) {
    m_acceleration = a;
    m_jerk = m_acceleration / m_timeToMaxA;
}

void SCurveProfile::setTimeToMaxA(double timeToMaxA) {
    m_timeToMaxA = timeToMaxA;
    m_jerk = m_acceleration / m_timeToMaxA;
}

