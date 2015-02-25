// =============================================================================
// File Name: TrapezoidProfile.cpp
// Description: Provides trapezoidal velocity control
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "TrapezoidProfile.hpp"
#include <cmath>

TrapezoidProfile::TrapezoidProfile(double maxV, double timeToMaxV) {
    setMaxVelocity(maxV);
    setTimeToMaxV(timeToMaxV);
}

TrapezoidProfile::~TrapezoidProfile() {
}

double TrapezoidProfile::updateSetpoint(double curTime, double curSource) {
    std::lock_guard<decltype(m_varMutex)> lock(m_varMutex);

    double tmpSP = 0.0;

    if (curTime < m_timeToMaxVelocity) {
        // Accelerate up
        tmpSP = (m_acceleration * curTime) * m_sign;
    }
    else if (curTime < m_timeFromMaxVelocity) {
        // Maintain max velocity
        tmpSP = m_profileMaxVelocity * m_sign;
    }
    else if (curTime < m_timeTotal) {
        // Accelerate down
        double decelTime = curTime - m_timeFromMaxVelocity;
        double v = m_profileMaxVelocity - m_acceleration * decelTime;

        tmpSP = v * m_sign;
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

double TrapezoidProfile::setGoal(double t, double goal, double curSource) {
    std::lock_guard<decltype(m_varMutex)> lock(m_varMutex);

    m_goal = goal;
    m_setpoint = goal - curSource;

    m_sign = (m_setpoint < 0) ? -1.0 : 1.0;
    m_timeToMaxVelocity = m_velocity / m_acceleration;

    /* d is distance traveled when accelerating to/from max velocity
     *       = 1/2 * (v0 + v) * t
     * t is m_timeToMaxVelocity
     * delta is distance travelled at max velocity
     * delta = totalDist - 2 * d
     *       = setpoint - 2 * ((v0 + v)/2 * t)
     * v0 = 0 therefore:
     * delta = setpoint - 2 * (v/2 * t)
     *       = setpoint - v * t
     *       = m_setpoint - m_velocity * m_timeToMaxVelocity
     *
     * t is time at maximum velocity
     * t = delta (from previous comment) / m_velocity (where m_velocity is maximum velocity)
     *   = (m_setpoint - m_velocity * m_timeToMaxVelocity) / m_velocity
     *   = m_setpoint/m_velocity - m_timeToMaxVelocity
     */
    double timeAtMaxV = m_sign * m_setpoint / m_velocity - m_timeToMaxVelocity;

    /* if ( 1/2 * a * t^2 > m_setpoint / 2 ) // if distance travelled before
     *     reaching maximum speed is more than half of the total distance to
     *     travel
     * if ( a * t^2 > m_setpoint )
     * if ( a * (v/a)^2 > m_setpoint )
     * if ( a * v^2/a^2 > m_setpoint )
     * if ( v^2/a > m_setpoint )
     * if ( v * v/a > m_setpoint )
     * if ( v * m_timeToMaxVelocity > m_setpoint )
     */
    if (m_velocity * m_timeToMaxVelocity > m_sign * m_setpoint) {
        /* Solve for t:
         * 1/2 * a * t^2 = m_setpoint/2
         * a * t^2 = m_setpoint
         * t^2 = m_setpoint / a
         * t = sqrt( m_setpoint / a )
         */
        m_timeToMaxVelocity = sqrt(m_sign * m_setpoint / m_acceleration);
        m_timeFromMaxVelocity = m_timeToMaxVelocity;
        m_timeTotal = 2 * m_timeToMaxVelocity;
        m_profileMaxVelocity = m_acceleration * m_timeToMaxVelocity;
    }
    else {
        m_timeFromMaxVelocity = m_timeToMaxVelocity + timeAtMaxV;
        m_timeTotal = m_timeFromMaxVelocity + m_timeToMaxVelocity;
        m_profileMaxVelocity = m_velocity;
    }

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

void TrapezoidProfile::setMaxVelocity(double v) {
    m_velocity = v;
}

double TrapezoidProfile::getMaxVelocity() const {
    return m_velocity;
}

void TrapezoidProfile::setTimeToMaxV(double timeToMaxV) {
    m_acceleration = m_velocity / timeToMaxV;
}

