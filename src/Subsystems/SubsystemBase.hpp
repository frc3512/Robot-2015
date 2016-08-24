// Copyright (c) FRC Team 3512, Spartatroniks 2015-2016. All Rights Reserved.

#ifndef SUBSYSTEM_BASE_HPP
#define SUBSYSTEM_BASE_HPP

#include "../Settings.hpp"

/**
 * Base class for all robot subsystems
 */
class SubsystemBase {
public:
    virtual ~SubsystemBase() = default;

    virtual void reloadPID() = 0;
    virtual void resetEncoders() = 0;

protected:
    Settings m_settings{"/home/lvuser/RobotSettings.txt"};
};

#endif  // SUBSYSTEM_BASE_HPP
