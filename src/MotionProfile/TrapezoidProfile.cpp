// =============================================================================
// File Name: TrapezoidProfile.cpp
// Description: Provides trapezoidal velocity control
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "TrapezoidProfile.hpp"
#include <iostream> // TODO Remove me

TrapezoidProfile::TrapezoidProfile( double maxV , double timeToMaxV ) {
    setMaxVelocity( maxV );
    setTimeToMaxV( timeToMaxV );
    setMode( SetpointMode::distance );
}

TrapezoidProfile::~TrapezoidProfile() {
}

double TrapezoidProfile::updateSetpoint( double curSetpoint ,
                                         double curSource ,
                                         double curTime ) {
    double setpoint = curSetpoint;
    double period = curTime - m_lastTime;

    m_varMutex.lock();

    if ( m_mode == SetpointMode::distance ) {
        if ( curTime < m_timeToMaxVelocity ) {
            // Accelerate up
            setpoint += ( m_acceleration * curTime ) * period * m_sign;
        }
        else if ( curTime < m_timeFromMaxVelocity ) {
            // Maintain max velocity
            setpoint += ( m_velocity * period * m_sign );
        }
        else if ( curTime < m_timeTotal ) {
            // Accelerate down
            double decelTime = curTime - m_timeFromMaxVelocity;
            double v = m_velocity + ( -m_acceleration * decelTime );

            setpoint += v * period * m_sign;
        }
    }
    else if ( m_mode == SetpointMode::velocity ) {
        if ( curTime < m_timeToMaxVelocity ) {
            // Accelerate up
            setpoint = ( m_acceleration * curTime ) * m_sign;
        }
        else if ( curTime < m_timeFromMaxVelocity ) {
            // Maintain max velocity
            setpoint = m_velocity * m_sign;
        }
        else if ( curTime < m_timeTotal ) {
            // Accelerate down
            double decelTime = curTime - m_timeFromMaxVelocity;
            double v = m_velocity + ( -m_acceleration * decelTime );

            setpoint = v * m_sign;
        }
    }

    m_varMutex.unlock();

    m_lastTime = curTime;
    return setpoint;
}

double TrapezoidProfile::setGoal( double goal , double curSource , double t ) {
    double setpoint = goal - curSource;

    m_sign = ( setpoint < 0 ) ? -1.0 : 1.0;
    m_timeToMaxVelocity = m_velocity / m_acceleration;

    m_varMutex.lock();

    double deltaPosMaxV = ( m_sign * setpoint ) -
                          ( m_timeToMaxVelocity * m_velocity );
    double timeAtMaxV = deltaPosMaxV / m_velocity;

    m_timeFromMaxVelocity = m_timeToMaxVelocity + timeAtMaxV;
    m_timeTotal = m_timeFromMaxVelocity + m_timeToMaxVelocity;

    m_varMutex.unlock();

    m_lastTime = t;

    if ( m_mode == SetpointMode::distance ) {
        // Set setpoint to current distance since setpoint hasn't moved yet
        return curSource;
    }
    else if ( m_mode == SetpointMode::velocity ) {
        // Set setpoint to zero since setpoint hasn't moved yet
        return 0.f;
    }
    else {
        return curSource;
    }
}

bool TrapezoidProfile::atGoal() {
    return m_lastTime > m_timeTotal;
}

void TrapezoidProfile::setMaxVelocity( double v ) {
    m_velocity = v;
}

void TrapezoidProfile::setTimeToMaxV( double timeToMaxV ) {
    m_acceleration = m_velocity / timeToMaxV;
}

void TrapezoidProfile::resetTime() {
    m_lastTime = 0.0;
}

void TrapezoidProfile::setMode( SetpointMode mode ) {
    m_mode = mode;
}

SetpointMode TrapezoidProfile::getMode() {
    return m_mode;
}

