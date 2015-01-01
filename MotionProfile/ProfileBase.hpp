//=============================================================================
//File Name: ProfileBase.hpp
//Description: Base class for all types of motion profile controllers
//Author: FRC Team 3512, Spartatroniks
//=============================================================================

#ifndef PROFILE_BASE_HPP
#define PROFILE_BASE_HPP

class ProfileBase {
public:
    ProfileBase();
    virtual ~ProfileBase();

    virtual double updateSetpoint( double curSetpoint , double curSource , double curTime ) = 0;
    virtual double setGoal( double goal , double curSource , double t ) = 0;
    virtual bool atGoal() = 0;
};

#endif // PROFILE_BASE_HPP
