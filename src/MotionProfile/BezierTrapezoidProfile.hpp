// =============================================================================
// File Name: BezierTrapezoidProfile.hpp
// Description: Provides trapezoidal velocity control and follows a given Bézier
//             curve
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

/* Implementation of trapezoid motion profile translated to C++; base Java code
 * courtesy of FRC Team 254; modifications for Bézier curve by FRC Team 3512.
 */

/* Constant acceleration until target (max) velocity is reached, sets
 * acceleration to zero for a calculated time, then decelerates at a constant
 * acceleration with a slope equal to the negative slope of the initial
 * acceleration.
 */

#ifndef BEZIER_TRAPEZOID_PROFILE_HPP
#define BEZIER_TRAPEZOID_PROFILE_HPP

#include "TrapezoidProfile.hpp"
#include "BezierCurve.hpp"

class BezierTrapezoidProfile : public TrapezoidProfile {
public:
    BezierTrapezoidProfile(double maxV, double timeToMaxV);

    /* curTime is current time
     *
     * returns updated uncompensated setpoint (see double getMidSetpoint())
     */
    double updateSetpoint(double curTime);

    /* Returns uncompensated setpoint for use in control of systems other than
     * the drive train
     */
    double getMidSetpoint() const;

    double getLeftSetpoint() const;
    double getRightSetpoint() const;

    /* goal is a Bézier curve for robot to follow
     * curSource is the current position
     * t initializes m_lastTime
     *
     * returns starting setpoint
     */
    double setCurveGoal(const BezierCurve& curve, double t);

    void resetProfile();

    // Sets distance between two sides of drive train
    void setWidth(double width);

private:
    // The robot follows this by turning in the motion profile
    BezierCurve m_curve;
    double m_width;

    // Collection of setpoints
    double m_leftSetpoint;
    double m_rightSetpoint;

    /* t is time elapsed since start of motion
     * v is current setpoint velocity of middle of robot
     */
    double getLeftVelocity(double t, double v) const;
    double getRightVelocity(double t, double v) const;
};

#endif // BEZIER_TRAPEZOID_PROFILE_HPP

