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

/* The default state is "IDLE". States that end the state machine should return
 * "IDLE" from advanceFunc()
 */

class StateMachine {
public:
    /* Pass in a state here if changing the IDLE state is needed. If it isn't
     * already named "IDLE", it will be renamed to that.
     */
    StateMachine(State* state);
    StateMachine(std::unique_ptr<State> state);
    StateMachine();

    virtual ~StateMachine();

    /* Ownership of 'state' will be transferred to this class, which will handle
     * destroying it.
     */
    void addState(State* state);
    void addState(std::unique_ptr<State> state);

    template <class... Args>
    void emplaceState(Args&& ... args);

    /* Moves the state machine to the given state. If the next state is found,
     * endFunc() for the current state and initFunc() for the next state. 'true'
     * is returned if the next state was found and 'false' otherwise.
     */
    bool setState(std::string nextState);

    // Returns name of current state
    std::string getState();

    // Sets starting state
    void setInitialState(std::string initState);

    // Call this periodically to operate the state machine
    void run();

    // Initiates the state machine (advances past initial state)
    void start();

    // Returns true if state machine is in initial state
    bool isStopped();

    // Stops running the state machine and resets it to the initial state
    void cancel();

private:
    std::vector<std::unique_ptr<State>> m_states;
    State* m_currentState{nullptr};
    std::string m_initStateName{"IDLE"};
};

#include "StateMachine.inl"

#endif // STATE_MACHINE_HPP

