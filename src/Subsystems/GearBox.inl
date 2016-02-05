// =============================================================================
// File Name: GearBox.inl
// Description: Represents a gear box with up to 3 motors and an encoder
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include <Encoder.h>
#include <PIDController.h>

template <class T>
GearBox<T>::GearBox(int shifterChan,
                    int encA,
                    int encB,
                    int motor1,
                    int motor2,
                    int motor3) :
    GearBoxBase<T>(shifterChan, encA, encB, motor1, motor2, motor3) {
    if (encA != -1 && encB != -1) {
        m_encoder = std::make_shared<Encoder>(encA, encB);
        m_pid =
            std::make_unique<PIDController>(0, 0, 0, 0, m_encoder.get(), this);

        m_encoder->SetPIDSourceType(PIDSourceType::kDisplacement);

        m_pid->SetAbsoluteTolerance(1);

        m_pid->Enable();
    }
    else {
        m_encoder = nullptr;
        m_pid = nullptr;
    }
}

template <class T>
void GearBox<T>::setSetpoint(float setpoint) {
    if (m_pid != nullptr) {
        if (!m_pid->IsEnabled()) {
            m_pid->Enable();
        }

        m_pid->SetSetpoint(setpoint);
    }
}

template <class T>
float GearBox<T>::getSetpoint() const {
    if (m_pid != nullptr) {
        return m_pid->GetSetpoint();
    }
    else {
        return 0.f;
    }
}

template <class T>
void GearBox<T>::setManual(float value) {
    if (m_pid != nullptr) {
        if (m_pid->IsEnabled()) {
            m_pid->Disable();
        }
    }

    PIDWrite(value);
}

template <class T>
float GearBox<T>::get(Grbx::PIDMode mode) const {
    if (mode == Grbx::Raw) {
        return GearBoxBase<T>::m_motors[0].Get();
    }
    else if (m_pid != nullptr) {
        if (mode == Grbx::Position) {
            return m_encoder->GetDistance();
        }
        else if (mode == Grbx::Speed) {
            return m_encoder->GetRate();
        }
    }

    return 0.f;
}

template <class T>
void GearBox<T>::setPID(float p, float i, float d) {
    if (m_pid != nullptr) {
        m_pid->SetPID(p, i, d);
    }
}

template <class T>
void GearBox<T>::setF(float f) {
    if (m_pid != nullptr) {
        GearBoxBase<T>::m_feedforward = f;
    }
}

template <class T>
void GearBox<T>::setDistancePerPulse(double distancePerPulse) {
    if (m_pid != nullptr) {
        GearBoxBase<T>::m_distancePerPulse = distancePerPulse;
        m_encoder->SetDistancePerPulse(distancePerPulse);
    }
}

template <class T>
void GearBox<T>::resetEncoder() {
    if (m_pid != nullptr) {
        m_encoder->Reset();
    }
}

template <class T>
void GearBox<T>::setEncoderReversed(bool reverse) {
    GearBoxBase<T>::m_isEncoderReversed = reverse;
    m_encoder->SetReverseDirection(reverse);
}

template <class T>
bool GearBox<T>::onTarget() const {
    if (m_pid != nullptr) {
        return m_pid->OnTarget();
    }
    else {
        return false;
    }
}

template <class T>
void GearBox<T>::resetPID() {
    m_pid->Reset();
    m_pid->Enable();
}

template <class T>
void GearBox<T>::setPIDSourceType(PIDSourceType pidSource) {
    if (m_pid != nullptr) {
        m_encoder->SetPIDSourceType(pidSource);
    }
}

template <class T>
void GearBox<T>::PIDWrite(float output) {
    for (auto& motor : GearBoxBase<T>::m_motors) {
        motor->Set(output + GearBoxBase<T>::m_feedforward);
    }
}

