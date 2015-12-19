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
    explicit State(std::string name);

    const std::string& name() const;

    // entry() is run when the state is first transitioned to.
    std::function<void()> entry = [] {};

    /* transition() transitions the state of the state machine to the state
     * which has the name returned. If "" is returned, the current state will be
     * maintained.
     */
    std::function<std::string()> transition = [] { return ""; };

    // run() is run while the state machine is in that state.
    std::function<void()> run = [] {};

    // exit() is run when the state is being transitioned away from.
    std::function<void()> exit = [] {};

private:
    // Contains the name of the state
    std::string m_name;
};

#endif // STATE_HPP

