// =============================================================================
// File Name: DSDisplay.inl
// Description: Receives IP address from remote host then sends HUD data there
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

template <class T>
void DSDisplay::addAutonMethod(const std::string& methodName,
                               void (T::* function)(),
                               T* object) {
    m_autonModes.addMethod(methodName, std::bind(function, object));
}

