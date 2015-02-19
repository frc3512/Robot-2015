// =============================================================================
// File Name: StateMachine.cpp
// Description: Provides an easier way to create state machines
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "StateMachine.hpp"

void StateMachine::addState(State* state) {
    m_states.push_back(std::unique_ptr<State>(state));
}

void StateMachine::addState(std::unique_ptr<State> state) {
    m_states.push_back(std::move(state));
}

std::string StateMachine::getState() {
    return m_states[m_currentState]->name;
}

void StateMachine::run() {
    m_states[m_currentState]->periodicFunc();

    if (m_states[m_currentState]->advanceFunc()) {
        m_states[m_currentState]->endFunc();
        m_currentState++;

        if (m_currentState == m_states.size()) {
            m_currentState = 0;
        }
    }
}

void StateMachine::start() {
    if (m_currentState == 0) {
        m_currentState++;

        if (m_currentState == m_states.size()) {
            m_currentState = 0;
        }
    }
}

bool StateMachine::isStopped() {
    return m_currentState == 0;
}

void StateMachine::cancel() {
    m_states[m_currentState]->endFunc();
    m_currentState = 0;
}

