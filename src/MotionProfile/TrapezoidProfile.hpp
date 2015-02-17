// =============================================================================
// File Name: TrapezoidProfile.hpp
// Description: Provides trapezoidal velocity control
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

/* Implementation of trapezoid motion profile translated to C++; base Java code
 * courtesy of FRC Team 254
 */

/* Constant acceleration until target (max) velocity is reached, sets
 * acceleration to zero for a calculated time, then decelerates at a constant
 * acceleration with a slope equal to the negative slope of the initial
 * acceleration.
 */

#ifndef TRAPEZOID_PROFILE_HPP
#define TRAPEZOID_PROFILE_HPP

#include "ProfileBase.hpp"
#include <mutex>

class TrapezoidProfile : public ProfileBase {
public:
    TrapezoidProfile(double maxV, double timeToMaxV);
    virtual ~TrapezoidProfile();

    /* If distance:
     *     curSetpoint is current distance set to which to travel
     *     curSource is current position (not used)
     *
     * If velocity:
     *     curSetpoint is current velocity at which to travel
     *     curSource is current velocity (not used)
     *
     * curTime is current time
     */
    virtual double updateSetpoint(double curTime, double curSource = 0.0);

    /* goal is a distance to which to travel
     * curSource is the current position
     * t initializes m_lastTime
     */
    virtual double setGoal(double t, double goal, double curSource = 0.0);

    bool atGoal();
    double getGoal() const;

    double getSetpoint() const;

    void setMaxVelocity(double v);
    double getMaxVelocity() const;
    void setTimeToMaxV(double timeToMaxV);

    virtual void resetProfile();

    // Tells algorithm whether to use distance or velocity as setpoint
    void setMode(SetpointMode mode);
    SetpointMode getMode() const;

protected:
    double m_setpoint;

    double m_lastTime;
    double m_acceleration;
    double m_velocity;
    double m_profileMaxVelocity;
    double m_timeFromMaxVelocity;
    double m_timeToMaxVelocity;
    double m_sign;
    double m_timeTotal;
    double m_goal;

    SetpointMode m_mode;

    std::recursive_mutex m_varMutex;
};

#endif // TRAPEZOID_PROFILE_HPP

