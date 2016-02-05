// =============================================================================
// File Name: State.cpp
// Description: Defines State in StateMachine class
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "State.hpp"

State::State(std::string name) : m_name{std::move(name)} {
}

const std::string& State::Name() const {
    return m_name;
}

