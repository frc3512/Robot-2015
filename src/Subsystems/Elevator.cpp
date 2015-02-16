// =============================================================================
// File Name: Elevator.cpp
// Description: Provides an interface for the robot's elevator
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "Elevator.hpp"
#include <Solenoid.h>
#include <DigitalInput.h>
#include <CANTalon.h>

Elevator::Elevator() {
    m_grabSolenoid = std::make_unique<Solenoid>(3);

    m_bottomLimit = std::make_unique<DigitalInput>(2);
    m_bottomLimit->RequestInterrupts(resetEncoder, m_liftGrbx.get());
    m_bottomLimit->SetUpSourceEdge(true, true);
    m_bottomLimit->EnableInterrupts();

    m_intakeVertical = std::make_unique<Solenoid>(1);
    m_intakeGrabber = std::make_unique<Solenoid>(2);
    m_intakeWheelLeft = std::make_unique<CANTalon>(3);
    m_intakeWheelRight = std::make_unique<CANTalon>(6);
    m_settings = std::make_unique<Settings>("/home/lvuser/RobotSettings.txt");
    m_intakeState = S_STOPPED;
    m_manual = false;

#if 0
    // For WPILib PID loop
    m_liftGrbx = std::make_unique<GearBox<CANTalon>>(-1, 2, 3, 2, 7);

    /* gear ratio is 48 driver to 26 driven from output of gearbox (where
     * encoder shaft is located), therefore:
     * distance per pulse = 26/48/(number of pulses per revolution)
     *                    = 26/48/360
     *                    = 26/(48*360)
     */
    m_liftGrbx->setDistancePerPulse((70.5 / 1.92442) * (26.0 / (48.0 * 360.0)));
#else
    // For CANTalon PID loop
    m_liftGrbx = std::make_unique<GearBox<CANTalon>>(-1, true, 7, 2);
    m_liftGrbx->setDistancePerPulse(70.5 / 5090.0);

    m_liftGrbx->setProfile(false);
    m_liftGrbx->setIZone(50);
    m_liftGrbx->setCloseLoopRampRate(1.0);

    m_liftGrbx->setProfile(true);
    m_liftGrbx->setIZone(50);
	m_liftGrbx->setCloseLoopRampRate(1.0);
    // m_liftGrbx->setSoftPositionLimits(70.5, 0.0);
#endif

    reloadPID();
}

Elevator::~Elevator() {
}

void Elevator::elevatorGrab(bool state) {
    m_grabSolenoid->Set(!state);
}

bool Elevator::getElevatorGrab() {
    return !m_grabSolenoid->Get();
}

void Elevator::intakeGrab(bool state) {
    m_intakeGrabber->Set(state);
}

bool Elevator::getIntakeGrab() {
    return m_intakeGrabber->Get();
}

void Elevator::stowIntake(bool state) {
    m_intakeVertical->Set(state);
}

bool Elevator::isIntakeStowed() {
    return m_intakeVertical->Get();
}

void Elevator::setIntakeDirection(IntakeMotorState state) {
    m_intakeState = state;

    if (state == S_STOPPED) {
        m_intakeWheelLeft->Set(0);
        m_intakeWheelRight->Set(0);
    }
    else if (state == S_FORWARD) {
        m_intakeWheelLeft->Set(1);
        m_intakeWheelRight->Set(-1);
    }
    else if (state == S_REVERSED) {
        m_intakeWheelLeft->Set(-1);
        m_intakeWheelRight->Set(1);
    }
}

Elevator::IntakeMotorState Elevator::getIntakeDirection() {
    return m_intakeState;
}

void Elevator::setManualLiftSpeed(float value) {
    if (m_manual == true) {
        m_liftGrbx->setManual(value);
    }
}

void Elevator::setManualMode(bool on) {
    m_manual = on;
}

bool Elevator::isManualMode() {
    return m_manual;
}

void Elevator::setHeight(float height) {
    if (m_manual == false) {
        m_liftGrbx->setSetpoint(height);
    }
}

float Elevator::getHeight() {
    return m_liftGrbx->get(Grbx::Position);
}

float Elevator::getSetpoint() {
    if (!m_manual) {
        return m_liftGrbx->getSetpoint();
    }
    else {
        return 0;
    }
}

void Elevator::reloadPID() {
    m_settings->update();

    // First profile
    float p0 = 0.f;
    float i0 = 0.f;
    float d0 = 0.f;
    float f0 = 0.f;

    // Set elevator PID
    p0 = m_settings->getDouble("PID_ELEVATOR_DOWN_P");
    i0 = m_settings->getDouble("PID_ELEVATOR_DOWN_I");
    d0 = m_settings->getDouble("PID_ELEVATOR_DOWN_D");
    f0 = m_settings->getDouble("PID_ELEVATOR_DOWN_F");

    m_liftGrbx->setProfile(false);
    m_liftGrbx->setPID(p0, i0, d0);
    m_liftGrbx->setF(f0);

    // Second profile
    float p1 = 0.f;
    float i1 = 0.f;
    float d1 = 0.f;
    float f1 = 0.f;

    // Set elevator PID
    p1 = m_settings->getDouble("PID_ELEVATOR_UP_P");
    i1 = m_settings->getDouble("PID_ELEVATOR_UP_I");
    d1 = m_settings->getDouble("PID_ELEVATOR_UP_D");
    f1 = m_settings->getDouble("PID_ELEVATOR_UP_F");

    m_liftGrbx->setProfile(true);
    m_liftGrbx->setPID(p1, i1, d1);
    m_liftGrbx->setF(f1);
}

bool Elevator::onTarget() {
    return m_liftGrbx->onTarget();
}

void Elevator::resetEncoder() {
    resetEncoder(0, m_liftGrbx.get());
}

void Elevator::resetEncoder(uint32_t interruptAssertedMask, void* param) {
    reinterpret_cast<decltype(m_liftGrbx.get())>(param)->resetEncoder();
}

void Elevator::pollLimitSwitch() {
    // Check encoder reset limit switch
    if (m_liftGrbx->isRevLimitSwitchClosed()) {
        m_liftGrbx->resetEncoder();
    }
}

