// =============================================================================
// File Name: TrapezoidProfile.cpp
// Description: Provides trapezoidal velocity control
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "TrapezoidProfile.hpp"
#include <cmath>

TrapezoidProfile::TrapezoidProfile(double maxV, double timeToMaxV) :
    m_setpoint(0.0) {
    setMaxVelocity(maxV);
    setTimeToMaxV(timeToMaxV);
    setMode(SetpointMode::distance);
    resetProfile();
}

TrapezoidProfile::~TrapezoidProfile() {
}

void TrapezoidProfile::manualChangeSetpoint(double delta) {
		m_setpoint += delta;
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
            m_setpoint += (m_profileMaxVelocity * period * m_sign);
        }
        else if (curTime < m_timeTotal) {
            // Accelerate down
            double decelTime = curTime - m_timeFromMaxVelocity;
            double v = m_profileMaxVelocity - m_acceleration * decelTime;

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
            m_setpoint = m_profileMaxVelocity * m_sign;
        }
        else if (curTime < m_timeTotal) {
            // Accelerate down
            double decelTime = curTime - m_timeFromMaxVelocity;
            double v = m_profileMaxVelocity - m_acceleration * decelTime;

            m_setpoint = v * m_sign;
        }
    }

    m_varMutex.unlock();

    m_lastTime = curTime;
    return m_setpoint;
}

double TrapezoidProfile::setGoal(double t, double goal, double curSource) {
    m_varMutex.lock();

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

    m_varMutex.unlock();

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

bool TrapezoidProfile::atGoal() {
    return m_lastTime >= m_timeTotal;
}

void TrapezoidProfile::setMaxVelocity(double v) {
    m_velocity = v;
}

void TrapezoidProfile::setTimeToMaxV(double timeToMaxV) {
    m_acceleration = m_velocity / timeToMaxV;
}

void TrapezoidProfile::resetProfile() {
    m_lastTime = 0.0;
    m_setpoint = 0.0;
}

void TrapezoidProfile::setMode(SetpointMode mode) {
    m_mode = mode;
}

SetpointMode TrapezoidProfile::getMode() const {
    return m_mode;
}

double TrapezoidProfile::getGoal() const {
	return m_goal;
}

double TrapezoidProfile::getMaxVelocity() const {
	return m_velocity;
}
