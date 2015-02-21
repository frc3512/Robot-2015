// =============================================================================
// File Name: StateMachine.inl
// Description: Provides an easier way to create state machines
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

template <class... Args>
void StateMachine::emplaceState(Args&& ... args) {
    m_states.push_back(std::make_unique<State>(std::forward<Args>(args) ...));
}

