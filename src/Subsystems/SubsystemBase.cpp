// =============================================================================
// File Name: SubsystemBase.cpp
// Description: Base class for all robot subsystems
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "SubsystemBase.hpp"

SubsystemBase::SubsystemBase() {
    m_settings = std::make_unique<Settings>("/home/lvuser/RobotSettings.txt");
}

SubsystemBase::~SubsystemBase() {
}

