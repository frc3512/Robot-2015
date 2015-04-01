// =============================================================================
// File Name: GraphHost.inl
// Description: The host for the LiveGrapher real-time graphing application
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

template <typename Rep, typename Period>
void GraphHost::setSendInterval(const duration<Rep, Period>& time) {
    m_sendInterval = duration_cast<milliseconds>(time).count();
}

