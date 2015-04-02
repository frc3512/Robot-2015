// =============================================================================
// File Name: GearBoxBase.inl
// Description: Represents a gear box with up to 3 motors and an encoder
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include <Solenoid.h>

template <class T>
GearBoxBase<T>::GearBoxBase(int shifterChan,
                            int encA,
                            int encB,
                            int motor1,
                            int motor2,
                            int motor3) {
    (void) encA;
    (void) encB;

    if (shifterChan != -1) {
        m_shifter = std::make_unique<Solenoid>(shifterChan);
    }
    else {
        m_shifter = nullptr;
    }

    m_isMotorReversed = false;
    m_isEncoderReversed = false;
    m_distancePerPulse = 1.f;

    // Create motor controllers of specified template type
    m_motors.emplace_back(std::make_unique<T>(motor1));
    if (motor2 != -1) {
        m_motors.emplace_back(std::make_unique<T>(motor2));
    }
    if (motor3 != -1) {
        m_motors.emplace_back(std::make_unique<T>(motor3));
    }

    m_setpoint = 0;
    m_feedforward = 0;
}

template <class T>
void GearBoxBase<T>::setMotorReversed(bool reverse) {
    m_isMotorReversed = reverse;
}

template <class T>
bool GearBoxBase<T>::isMotorReversed() const {
    return m_isMotorReversed;
}

template <class T>
bool GearBoxBase<T>::isEncoderReversed() const {
    return m_isEncoderReversed;
}

template <class T>
void GearBoxBase<T>::setGear(bool gear) {
    if (m_shifter != nullptr) {
        m_shifter->Set(gear);
    }
}

template <class T>
bool GearBoxBase<T>::getGear() const {
    if (m_shifter != nullptr) {
        return m_shifter->Get();
    }
    else {
        return false;
    }
}

