// =============================================================================
// Description: The host for the LiveGrapher real-time graphing application
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

template <typename Rep, typename Period>
void GraphHost::SetSendInterval(const duration<Rep, Period>& time) {
    m_sendInterval = duration_cast<milliseconds>(time).count();
}

