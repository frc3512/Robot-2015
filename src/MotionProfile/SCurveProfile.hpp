// Copyright (c) FRC Team 3512, Spartatroniks 2015-2016. All Rights Reserved.

/* Implementation of trapezoid motion profile translated to C++; base Java code
 * courtesy of FRC Team 254
 */

/* Constant acceleration until target (max) velocity is reached, sets
 * acceleration to zero for a calculated time, then decelerates at a constant
 * acceleration with a slope equal to the negative slope of the initial
 * acceleration.
 */

#pragma once

#include "ProfileBase.hpp"

/**
 * Provides trapezoidal acceleration control
 */
class SCurveProfile : public ProfileBase {
public:
    SCurveProfile(double maxV, double maxA, double timeToMaxA);

    // curTime is current time
    virtual double updateSetpoint(double curTime);

    /* goal is a distance to which to travel
     * curSource is the current position
     * t initializes m_lastTime
     */
    virtual double setGoal(double t, double goal, double curSource = 0.0);

    void setMaxVelocity(double v);
    double getMaxVelocity() const;
    void setMaxAcceleration(double a);
    void setTimeToMaxA(double timeToMaxA);

protected:
    double m_acceleration;
    double m_maxVelocity;
    double m_profileMaxVelocity;
    double m_timeToMaxA;

    double m_jerk;
    double m_t2;
    double m_t3;
    double m_t4;
    double m_t5;
    double m_t6;
    double m_t7;

    double m_sign;
};
