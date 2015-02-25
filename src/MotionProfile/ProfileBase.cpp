// =============================================================================
// File Name: ProfileBase.cpp
// Description: Base class for all types of motion profile controllers
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "ProfileBase.hpp"

ProfileBase::ProfileBase() {
    setMode(SetpointMode::distance);
    resetProfile();
}

ProfileBase::~ProfileBase() {
}

bool ProfileBase::atGoal() {
    return m_lastTime >= m_timeTotal;
}

double ProfileBase::getGoal() const {
    return m_goal;
}

double ProfileBase::getSetpoint() const {
    return m_setpoint;
}

void ProfileBase::resetProfile() {
    m_goal = 0.0;
    m_setpoint = 0.0;
    m_lastTime = 0.0;
    m_timeTotal = 0.0;
}

void ProfileBase::setMode(SetpointMode mode) {
    m_mode = mode;
}

SetpointMode ProfileBase::getMode() const {
    return m_mode;
}

