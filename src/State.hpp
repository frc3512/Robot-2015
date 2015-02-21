// =============================================================================
// File Name: State.hpp
// Description: Defines State in StateMachine class
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#ifndef STATE_HPP
#define STATE_HPP

#include <string>
#include <functional>

class State {
public:
    /* 'name' contains the name of the state.
     * initFunc() is run when the state is first transitioned to.
     * advanceFunc() advancees the state of the state machine to the state
     *     which has the name returned. If "" is returned, the current state
     *     will be maintained.
     * periodicFunc() is run while the state machine is in that state.
     * endFunc() is run when the state is being transitioned away from.
     */
    State(std::string name, std::function<void()> initFunc = [] {
    }, std::function<std::string()> advanceFunc =
            [] { return ""; },
          std::function<void()> periodicFunc = [] {},
          std::function<void()> endFunc = [] {});

    std::string name;
    std::function<void()> initFunc;
    std::function<std::string()> advanceFunc;
    std::function<void()> periodicFunc;
    std::function<void()> endFunc;
};

#endif // STATE_HPP

