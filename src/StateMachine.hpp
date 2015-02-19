// =============================================================================
// File Name: StateMachine.hpp
// Description: Provides an easier way to create state machines
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#ifndef STATE_MACHINE_HPP
#define STATE_MACHINE_HPP

#include <memory>
#include <vector>
#include "State.hpp"

class StateMachine {
public:
    /* Ownership of 'state' will be transferred to this class, which will handle
     * destroying it.
     */
    void addState(State* state);
    void addState(std::unique_ptr<State> state);

    // Returns name of current state
    std::string getState();

    // Call this periodically to operate the state machine
    void run();

    // Initiates state machine (advances past initial state)
    void start();

    // Returns true if state machine is in initial state
    bool isStopped();

    // Stops running the state machine and resets it to the initial state
    void cancel();

private:
    std::vector<std::unique_ptr<State>> m_states;
    size_t m_currentState{0};
};

#endif // STATE_MACHINE_HPP

