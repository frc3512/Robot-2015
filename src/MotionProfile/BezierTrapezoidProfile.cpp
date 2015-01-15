//=============================================================================
//File Name: BezierTrapezoidProfile.cpp
//Description: Provides trapezoidal velocity control and follows a given Bézier
//             curve
//Author: FRC Team 3512, Spartatroniks
//=============================================================================

#include "BezierTrapezoidProfile.hpp"

BezierTrapezoidProfile::BezierTrapezoidProfile( double maxV , double timeToMaxV ) :
        TrapezoidProfile( maxV , timeToMaxV ) {
    setMaxVelocity(maxV);
    setTimeToMaxV(timeToMaxV);
    setMode( SetpointMode::distance );

    m_width = 0.0;
    m_leftSetpoint = 0.0;
    m_rightSetpoint = 0.0;
}

BezierTrapezoidProfile::~BezierTrapezoidProfile() {

}

double BezierTrapezoidProfile::updateSetpoint( double curTime , double curSource ) {
    double period = curTime - m_lastTime;

    m_varMutex.lock();

    if ( m_mode == SetpointMode::distance ) {
        if ( curTime < m_timeToMaxVelocity ) {
            // Accelerate up
            m_setpoint += (m_acceleration * curTime) * period * m_sign;
            m_leftSetpoint += getLeftVelocity( curTime , m_acceleration * curTime ) * period * m_sign;
            m_rightSetpoint += getRightVelocity( curTime , m_acceleration * curTime ) * period * m_sign;
        }
        else if ( curTime < m_timeFromMaxVelocity ) {
            // Maintain max velocity
            m_setpoint += ( m_velocity * period * m_sign );
            m_leftSetpoint += getLeftVelocity( curTime , m_velocity ) * period * m_sign;
            m_rightSetpoint += getRightVelocity( curTime , m_velocity ) * period * m_sign;
        }
        else if ( curTime < m_timeTotal ) {
            // Accelerate down
            double decelTime = curTime - m_timeFromMaxVelocity;
            double v = m_velocity + (-m_acceleration * decelTime);

            m_setpoint += v * period * m_sign;
            m_leftSetpoint += getLeftVelocity( curTime , v ) * period * m_sign;
            m_rightSetpoint += getRightVelocity( curTime , v ) * period * m_sign;
        }
    }
    else if ( m_mode == SetpointMode::velocity ) {
        if ( curTime < m_timeToMaxVelocity ) {
            // Accelerate up
            m_setpoint = (m_acceleration * curTime) * m_sign;
            m_leftSetpoint = getLeftVelocity( curTime , m_acceleration * curTime ) * m_sign;
            m_rightSetpoint = getRightVelocity( curTime , m_acceleration * curTime ) * m_sign;
        }
        else if ( curTime < m_timeFromMaxVelocity ) {
            // Maintain max velocity
            m_setpoint = m_velocity * m_sign;
            m_leftSetpoint = getLeftVelocity( curTime , m_velocity ) * m_sign;
            m_rightSetpoint = getRightVelocity( curTime , m_velocity ) * m_sign;
        }
        else if ( curTime < m_timeTotal ) {
            // Accelerate down
            double decelTime = curTime - m_timeFromMaxVelocity;
            double v = m_velocity + (-m_acceleration * decelTime);

            m_setpoint = v * m_sign;
            m_leftSetpoint = getLeftVelocity( curTime , v ) * m_sign;
            m_rightSetpoint = getRightVelocity( curTime , v ) * m_sign;
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

double BezierTrapezoidProfile::setGoal( const BezierCurve& curve , double t ) {
    m_curve = curve;

    return setGoal( t , m_curve.getArcLength() );
}

void BezierTrapezoidProfile::setWidth( double width ) {
    m_width = width;
}

double BezierTrapezoidProfile::setGoal( double t , double goal , double curSource ) {
    m_varMutex.lock();

    m_setpoint = goal - curSource;

    m_sign = (m_setpoint < 0) ? -1.0 : 1.0;
    m_timeToMaxVelocity = m_velocity / m_acceleration;

    /* d is distance traveled when accelerating to/from max velocity
     *       = 1/2 * (v0 + v) * t
     * t is m_timeToMaxVelocity
     * delta is distance travelled at max velocity
     * delta = totalDist - 2 * d
     *       = (setpoint) - 2 * ((v0 + v)/2 * t)
     * v0 = 0 therefore:
     * delta = setpoint - 2 * (v/2 * t)
     *       = setpoint - v * t
     */
    double deltaPosMaxV = (m_sign * m_setpoint) - (m_timeToMaxVelocity * m_velocity);
    double timeAtMaxV = deltaPosMaxV / m_velocity;

    m_timeFromMaxVelocity = m_timeToMaxVelocity + timeAtMaxV;
    m_timeTotal = m_timeFromMaxVelocity + m_timeToMaxVelocity;

    m_varMutex.unlock();

    m_lastTime = t;

    if ( m_mode == SetpointMode::distance ) {
        // Set setpoint to current distance since setpoint hasn't moved yet
        return m_setpoint = curSource;
    }
    else if ( m_mode == SetpointMode::velocity ) {
        // Set setpoint to zero since setpoint hasn't moved yet
        return m_setpoint = 0.f;
    }
    else {
        return curSource;
    }
}

double BezierTrapezoidProfile::getLeftVelocity( double t , double v ) const {
    return ( 1.0 - m_curve.getCurvature( t/m_timeTotal ) * m_width / 2.0 ) * v;
}

double BezierTrapezoidProfile::getRightVelocity( double t , double v ) const {
    return ( 1.0 + m_curve.getCurvature( t/m_timeTotal ) * m_width / 2.0 ) * v;
}
