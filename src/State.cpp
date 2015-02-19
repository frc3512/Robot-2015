// =============================================================================
// File Name: State.cpp
// Description: Defines State in StateMachine class
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "State.hpp"

State::State(std::string name, std::function<void()> initFunc,
             std::function<bool()> advanceFunc,
             std::function<void()> periodicFunc,
             std::function<void()> endFunc) {
    this->name = name;
    this->initFunc = initFunc;
    this->advanceFunc = advanceFunc;
    this->periodicFunc = periodicFunc;
    this->endFunc = endFunc;
}

