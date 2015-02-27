// =============================================================================
// File Name: SubsystemBase.hpp
// Description: Base class for all robot subsystems
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#ifndef SUBSYSTEM_BASE_HPP
#define SUBSYSTEM_BASE_HPP

#include <memory>

#include "../Settings.hpp"

class SubsystemBase {
public:
    SubsystemBase();
    virtual ~SubsystemBase();

    virtual void reloadPID() = 0;
    virtual void resetEncoders() = 0;

protected:
    std::unique_ptr<Settings> m_settings;
};

#endif // SUBSYSTEM_BASE_HPP
