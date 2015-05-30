// =============================================================================
// File Name: SubsystemBase.hpp
// Description: Base class for all robot subsystems
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#ifndef SUBSYSTEM_BASE_HPP
#define SUBSYSTEM_BASE_HPP

#include "../Settings.hpp"

class SubsystemBase {
public:
    virtual ~SubsystemBase() = default;

    virtual void reloadPID() = 0;
    virtual void resetEncoders() = 0;

protected:
    Settings m_settings{"/home/lvuser/RobotSettings.txt"};
};

#endif // SUBSYSTEM_BASE_HPP

