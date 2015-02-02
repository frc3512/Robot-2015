// =============================================================================
// File Name: ProfileBase.hpp
// Description: Base class for all types of motion profile controllers
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#ifndef PROFILE_BASE_HPP
#define PROFILE_BASE_HPP

typedef enum {
    distance,
    velocity
} SetpointMode;

class ProfileBase {
public:
    ProfileBase();
    virtual ~ProfileBase();

    virtual double updateSetpoint(double curTime, double curSource = 0.0) = 0;

    // Should return initial setpoint for start of profile
    virtual double setGoal(double t, double goal, double curSource) = 0;
    virtual bool atGoal() = 0;
};

#endif // PROFILE_BASE_HPP

