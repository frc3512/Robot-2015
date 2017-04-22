// Copyright (c) FRC Team 3512, Spartatroniks 2015-2017. All Rights Reserved.

#pragma once

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
