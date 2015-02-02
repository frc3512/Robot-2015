// =============================================================================
// File Name: GearBoxCANTalon.inl
// Description: Represents a gear box with up to 3 motors and an encoder
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include <Encoder.h>
#include <Solenoid.h>

inline GearBox<CANTalon>::GearBox(int shifterChan,
                                  int motor1,
                                  int motor2,
                                  int motor3) {
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
    m_motors.emplace_back(std::make_unique<CANTalon>(motor1));
    if (motor2 != -1) {
        m_motors.emplace_back(std::make_unique<CANTalon>(motor2));
    }
    if (motor3 != -1) {
        m_motors.emplace_back(std::make_unique<CANTalon>(motor3));
    }

    for (unsigned int i = 0; i < m_motors.size(); i++) {
        if (i == 0) {
            m_motors[i]->SetControlMode(CANTalon::kPercentVbus);
        }
        else {
            m_motors[i]->SetControlMode(CANTalon::kFollower);
        }
        m_motors[i]->SetFeedbackDevice(CANTalon::QuadEncoder);
        m_motors[i]->EnableControl();
    }
}

inline void GearBox<CANTalon>::setSetpoint(float setpoint) {
    for (auto& motor : m_motors) {
        motor->SetControlMode(CANTalon::kPosition);

        if (!m_isMotorReversed) {
            motor->Set(setpoint / m_distancePerPulse);
        }
        else {
            motor->Set(-setpoint / m_distancePerPulse);
        }
    }
}

inline void GearBox<CANTalon>::setManual(float value) {
    for (auto& motor : m_motors) {
        motor->SetControlMode(CANTalon::kPercentVbus);

        if (!m_isMotorReversed) {
            motor->Set(value);
        }
        else {
            motor->Set(-value);
        }
    }
}

inline float GearBox<CANTalon>::get() {
    return m_motors[0]->Get() * m_distancePerPulse;
}

inline void GearBox<CANTalon>::setPID(float p, float i, float d) {
    for (auto& motor : m_motors) {
        motor->SetPID(p, i, d);
    }
}

inline void GearBox<CANTalon>::setF(float f) {
    for (auto& motor : m_motors) {
        motor->SetF(f);
    }
}

inline void GearBox<CANTalon>::setDistancePerPulse(double distancePerPulse) {
    m_distancePerPulse = distancePerPulse;
}

inline void GearBox<CANTalon>::setControlMode(CANTalon::ControlMode ctrlMode) {
    for (auto& motor : m_motors) {
        motor->SetControlMode(ctrlMode);
    }
}

inline void GearBox<CANTalon>::resetEncoder() {
    for (auto& motor : m_motors) {
        motor->SetNumberOfQuadIdxRises(0);
    }
}

inline double GearBox<CANTalon>::getDistance() {
    return m_motors[0]->GetEncPosition();
}

inline double GearBox<CANTalon>::getRate() {
    return m_motors[0]->GetEncVel();
}

inline void GearBox<CANTalon>::setMotorReversed(bool reverse) {
    m_isMotorReversed = reverse;
}

inline bool GearBox<CANTalon>::isMotorReversed() const {
    return m_isMotorReversed;
}

inline void GearBox<CANTalon>::setEncoderReversed(bool reverse) {
    for (auto& motor : m_motors) {
        motor->SetSensorDirection(reverse);
    }
}

inline bool GearBox<CANTalon>::isEncoderReversed() const {
    return m_isEncoderReversed;
}

inline void GearBox<CANTalon>::setGear(bool gear) {
    if (m_shifter != nullptr) {
        m_shifter->Set(gear);
    }
}

inline bool GearBox<CANTalon>::getGear() const {
    if (m_shifter != nullptr) {
        return m_shifter->Get();
    }
    else {
        return false;
    }
}

inline bool GearBox<CANTalon>::onTarget() {
    return m_motors[0]->GetClosedLoopError() < 10;
}

inline void GearBox<CANTalon>::resetPID() {
    for (auto& motor : m_motors) {
        motor->ClearIaccum();
    }
}

