// =============================================================================
// File Name: ProfileBase.hpp
// Description: Base class for all types of motion profile controllers
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#ifndef PROFILE_BASE_HPP
#define PROFILE_BASE_HPP

#include <mutex>

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
    virtual bool atGoal();

    double getGoal() const;
    double getSetpoint() const;

    virtual void resetProfile();

    // Tells algorithm whether to use distance or velocity as setpoint
    void setMode(SetpointMode mode);
    SetpointMode getMode() const;

protected:
    // Use this to make updateSetpoint() and setGoal() thread-safe
    std::recursive_mutex m_varMutex;

    double m_goal;
    double m_setpoint;
    double m_lastTime;
    double m_timeTotal;

    SetpointMode m_mode;
};

#endif // PROFILE_BASE_HPP

