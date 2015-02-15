// =============================================================================
// File Name: GearBoxCANTalon.inl
// Description: Represents a gear box with up to 3 motors and an encoder
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include <Encoder.h>
#include <Solenoid.h>

inline GearBox<CANTalon>::GearBox(int shifterChan,
                                  bool reverseEncoder,
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
    m_isEncoderReversed = reverseEncoder;
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
            m_motors[i]->SetFeedbackDevice(CANTalon::QuadEncoder);
            m_motors[i]->ConfigEncoderCodesPerRev(1);
            m_motors[i]->SetSensorDirection(m_isEncoderReversed);
            resetEncoder();
            m_motors[i]->EnableControl();
        }
        else {
            // Use all other CANTalons as slaves
            m_motors[i]->SetControlMode(CANTalon::kFollower);

            // Set first CANTalon as master
            m_motors[i]->Set(motor1);
        }
    }
}

inline GearBox<CANTalon>::~GearBox() {
}

inline void GearBox<CANTalon>::setSetpoint(float setpoint) {
    m_motors[0]->SetControlMode(CANTalon::kPosition);

    if (!m_isMotorReversed) {
        m_setpoint = setpoint / m_distancePerPulse;
    }
    else {
        m_setpoint = -setpoint / m_distancePerPulse;
    }

    m_motors[0]->Set(m_setpoint);
}

inline float GearBox<CANTalon>::getSetpoint() {
    return m_setpoint * m_distancePerPulse;
}

inline void GearBox<CANTalon>::setManual(float value) {
    m_motors[0]->SetControlMode(CANTalon::kPercentVbus);

    if (!m_isMotorReversed) {
        m_motors[0]->Set(value);
    }
    else {
        m_motors[0]->Set(-value);
    }
}

inline float GearBox<CANTalon>::get(Grbx::PIDMode mode) {
    if (mode == Grbx::Position) {
        return m_motors[0]->GetPosition() * m_distancePerPulse;
    }
    else if (mode == Grbx::Speed) {
        return m_motors[0]->GetEncVel() * m_distancePerPulse;
    }
    else if (mode == Grbx::Raw) {
        if (!m_isMotorReversed) {
            return m_motors[0]->Get();
        }
        else {
            return -m_motors[0]->Get();
        }
    }
    else if (mode == Grbx::Position) {
        return m_motors[0]->GetEncPosition() / m_distancePerPulse;
    }
    else if (mode == Grbx::Speed) {
        return m_motors[0]->GetEncVel() * m_distancePerPulse;
    }

    return 0.f;
}

inline void GearBox<CANTalon>::setPID(float p, float i, float d) {
    m_motors[0]->SetPID(p, i, d);
}

inline void GearBox<CANTalon>::setF(float f) {
    m_motors[0]->SetF(f);
}

inline void GearBox<CANTalon>::setDistancePerPulse(double distancePerPulse) {
    m_distancePerPulse = distancePerPulse;
}

inline void GearBox<CANTalon>::setControlMode(CANTalon::ControlMode ctrlMode) {
    m_motors[0]->SetControlMode(ctrlMode);
}

inline void GearBox<CANTalon>::resetEncoder() {
    m_motors[0]->SetPosition(0);
}

inline void GearBox<CANTalon>::setSoftPositionLimits(double forwardLimit,
                                                     double backwardLimit) {
    m_motors[0]->ConfigSoftPositionLimits(forwardLimit, backwardLimit);
}

inline void GearBox<CANTalon>::setMotorReversed(bool reverse) {
    m_isMotorReversed = reverse;
}

inline bool GearBox<CANTalon>::isMotorReversed() const {
    return m_isMotorReversed;
}

inline void GearBox<CANTalon>::setEncoderReversed(bool reverse) {
    m_motors[0]->SetSensorDirection(reverse);
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
    m_motors[0]->ClearIaccum();
}

inline bool GearBox<CANTalon>::isFwdLimitSwitchClosed() {
    return m_motors[0]->IsFwdLimitSwitchClosed();
}

inline bool GearBox<CANTalon>::isRevLimitSwitchClosed() {
    return m_motors[0]->IsRevLimitSwitchClosed();
}
