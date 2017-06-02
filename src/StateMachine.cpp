// Copyright (c) 2015-2017 FRC Team 3512. All Rights Reserved.

#include "StateMachine.hpp"

#include <iostream>
#include <string>
#include <utility>

StateMachine::StateMachine(std::string name) : State(std::move(name)) {
    run = [this] {
        if (m_currentState == nullptr) {
            return;
        }

        m_currentState->run();

        std::string nextState = m_currentState->transition();

        if (nextState.size() != 0) {
            if (!SetState(nextState)) {
                // Failed to find state matching the returned name
                std::cout << "[" << nextState << "] is not a known state\n";
            }
        }
    };
}

void StateMachine::AddState(std::unique_ptr<State> state) {
    m_states.push_back(std::move(state));
}

bool StateMachine::SetState(const std::string& newState) {
    for (auto& i : m_states) {
        if (i->Name() == newState) {
            if (m_currentState != nullptr) {
                m_currentState->exit();
            }
            m_currentState = i.get();
            m_currentState->entry();

            return true;
        }
    }

    return false;
}

const std::string StateMachine::GetState() const {
    if (m_currentState != nullptr) {
        return m_currentState->Name();
    } else {
        return "";
    }
}
