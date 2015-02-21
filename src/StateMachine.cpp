// =============================================================================
// File Name: StateMachine.cpp
// Description: Provides an easier way to create state machines
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "StateMachine.hpp"
#include <iostream>

StateMachine::StateMachine(State* state) :
    StateMachine(std::unique_ptr<State>(state)) {
}

StateMachine::StateMachine(std::unique_ptr<State> state) {
    if (state->name != "IDLE") {
        state->name = "IDLE";
    }
    addState(std::move(state));
}

StateMachine::StateMachine() {
    addState(new State("IDLE"));
}

StateMachine::~StateMachine() {
    if (m_currentState != nullptr) {
        m_currentState->endFunc();
    }
}

void StateMachine::addState(State* state) {
    m_states.push_back(std::unique_ptr<State>(state));
}

void StateMachine::addState(std::unique_ptr<State> state) {
    m_states.push_back(std::move(state));
}

bool StateMachine::setState(std::string newState) {
    for (auto& i : m_states) {
        if (i->name == newState) {
            m_currentState->endFunc();
            m_currentState = i.get();
            m_currentState->initFunc();

            return true;
        }
    }

    return false;
}

std::string StateMachine::getState() {
    if (m_currentState != nullptr) {
        return m_currentState->name;
    }
    else {
        return "";
    }
}

void StateMachine::setInitialState(std::string initState) {
    m_initStateName = initState;
}

void StateMachine::run() {
    if (m_currentState == nullptr) {
        return;
    }

    m_currentState->periodicFunc();

    std::string nextState = m_currentState->advanceFunc();

    if (nextState.size() != 0) {
        if (!setState(nextState)) {
            // Failed to find state matching the returned name
            std::cout << "[" << nextState << "] is not a known state\n";
        }
    }
}

void StateMachine::start() {
    setState(m_initStateName);
}

bool StateMachine::isStopped() {
    return m_currentState->name == "IDLE";
}

void StateMachine::cancel() {
    setState("IDLE");
}
