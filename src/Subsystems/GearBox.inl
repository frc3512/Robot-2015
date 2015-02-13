// =============================================================================
// File Name: GearBox.inl
// Description: Represents a gear box with up to 3 motors and an encoder
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include <Encoder.h>
#include <Solenoid.h>

#include <PIDController.h>

template <class T>
GearBox<T>::GearBox(int shifterChan,
                    int encA,
                    int encB,
                    int motor1,
                    int motor2,
                    int motor3) {
    if (encA != -1 && encB != -1) {
        m_encoder = std::make_shared<Encoder>(encA, encB);
        m_pid =
            std::make_unique<PIDController>(0, 0, 0, 0, m_encoder.get(), this);

        m_havePID = true;
    }
    else {
        m_encoder = nullptr;
        m_pid = nullptr;

        m_havePID = false;
    }

    if (shifterChan != -1) {
        m_shifter = std::make_unique<Solenoid>(shifterChan);
    }
    else {
        m_shifter = nullptr;
    }

    m_distancePerPulse = 0;
    m_feedforward = 0;
    m_isMotorReversed = false;
    m_isEncoderReversed = false;

    // Create motor controllers of specified template type
    m_motors.emplace_back(std::make_unique<T>(motor1));
    if (motor2 != -1) {
        m_motors.emplace_back(std::make_unique<T>(motor2));
    }
    if (motor3 != -1) {
        m_motors.emplace_back(std::make_unique<T>(motor3));
    }
    if (m_havePID) {
        m_encoder->SetPIDSourceParameter(Encoder::kDistance);

        m_pid->SetAbsoluteTolerance(1);

        m_pid->Enable();
    }

}

template <class T>
GearBox<T>::~GearBox() {
}

template <class T>
void GearBox<T>::setSetpoint(float setpoint) {
    if (m_havePID) {
        if (!m_pid->IsEnabled()) {
            m_pid->Enable();
        }

        m_pid->SetSetpoint(setpoint);
    }
}

template <class T>
float GearBox<T>::getSetpoint() {
    if (m_havePID) {
        return m_pid->GetSetpoint();
    }
    else {
        return 0.f;
    }
}

template <class T>
void GearBox<T>::setManual(float value) {
    if (m_havePID) {
        if (m_pid->IsEnabled()) {
            m_pid->Disable();
        }
    }

    PIDWrite(value);
}

template <class T>
float GearBox<T>::get(Grbx::PIDMode mode) const {
    if (mode == Grbx::Raw) {
        if (!m_isMotorReversed) {
            return m_motors[0]->Get();
        }
        else {
            return -m_motors[0]->Get();
        }
    }
    else if (m_havePID) {
        if (mode == Grbx::Position) {
            return m_encoder->GetDistance();
        }
        else if (mode == Grbx::Speed) {
            return m_encoder->GetRate();
        }
        else {
            return 0.f;
        }
    }
    else {
        return 0.f;
    }
}

template <class T>
void GearBox<T>::setPID(float p, float i, float d) {
    if (m_havePID) {
        m_pid->SetPID(p, i, d);
    }
}

template <class T>
void GearBox<T>::setF(float f) {
    if (m_havePID) {
        m_feedforward = f;
    }
}

template <class T>
void GearBox<T>::setDistancePerPulse(double distancePerPulse) {
    if (m_havePID) {
        m_distancePerPulse = distancePerPulse;
        m_encoder->SetDistancePerPulse(distancePerPulse);
    }
}

template <class T>
void GearBox<T>::setPIDSourceParameter(PIDSource::PIDSourceParameter pidSource)
{
    if (m_havePID) {
        m_encoder->SetPIDSourceParameter(pidSource);
    }
}

template <class T>
void GearBox<T>::resetEncoder() {
    if (m_havePID) {
        m_encoder->Reset();
    }
}

template <class T>
void GearBox<T>::setMotorReversed(bool reverse) {
    m_isMotorReversed = reverse;
}

template <class T>
bool GearBox<T>::isMotorReversed() const {
    return m_isMotorReversed;
}

template <class T>
void GearBox<T>::setEncoderReversed(bool reverse) {
    m_encoder->SetReverseDirection(reverse);
}

template <class T>
bool GearBox<T>::isEncoderReversed() const {
    return m_isEncoderReversed;
}

template <class T>
void GearBox<T>::setGear(bool gear) {
    if (m_shifter != nullptr) {
        m_shifter->Set(gear);
    }
}

template <class T>
bool GearBox<T>::getGear() const {
    if (m_shifter != nullptr) {
        return m_shifter->Get();
    }
    else {
        return false;
    }
}

template <class T>
void GearBox<T>::PIDWrite(float output) {
    for (auto& motor : m_motors) {
        if (!m_isMotorReversed) {
            motor->Set(output + m_feedforward);
        }
        else {
            motor->Set(-(output + m_feedforward));
        }
    }
}

template <class T>
bool GearBox<T>::onTarget() {
    if (m_havePID) {
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
