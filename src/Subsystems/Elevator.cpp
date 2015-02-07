/*
 * Elevator.cpp
 *
 *  Created on: Jan 16, 2015
 *      Author: acf
 */

#include "Elevator.hpp"
#include "Solenoid.h"
#include "CANTalon.h"

Elevator::Elevator() {
    m_grabSolenoid = new Solenoid(1);
    m_intakeVertical = new Solenoid(2);
    m_intakeGrabber = new Solenoid(3);
    m_intakeWheels = new CANTalon(6);
    m_settings = new Settings("/home/lvuser/RobotSettings.txt");
    m_intakeState = S_STOPPED;
    m_manual = false;

    m_liftGrbx = new GearBox<CANTalon>(-1, 4, 5);
    /* gear ratio is 48 driver to 26 driven from output of gearbox (where
     * encoder shaft is located), therefore:
     * distance per pulse = 26/48/(number of pulses per revolution)
     *                    = 26/48/360
     *                    = 26/(48*360)
     */
    m_liftGrbx->setDistancePerPulse(26 / (48 * 360));
    reloadPID();
}

Elevator::~Elevator() {
    delete m_grabSolenoid;
    delete m_intakeVertical;
    delete m_intakeGrabber;
    delete m_intakeWheels;
    delete m_settings;
    delete m_liftGrbx;
}

void Elevator::elevatorGrab(bool state) {
    m_grabSolenoid->Set(state);
}

bool Elevator::getElevatorGrab() {
    return m_grabSolenoid->Get();
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
        m_intakeWheels->Set(0);
    }
    else if (state == S_FORWARD) {
        m_intakeWheels->Set(1);
    }
    else if (state == S_REVERSED) {
        m_intakeWheels->Set(-1);
    }
}

Elevator::IntakeMotorState Elevator::getIntakeDirection() {
    return m_intakeState;
}

void Elevator::stop(bool state) {
}

void Elevator::setManualLiftSpeed(float value) {
    if (m_manual == true) {
        m_liftGrbx->setManual(value);
    }
}

void Elevator::setManualMode(bool on) {
    m_manual = on;
}

bool Elevator::getManualMode() {
    return m_manual;
}

void Elevator::setHeight(float height) {
    if (m_manual == false) {
        m_liftGrbx->setSetpoint(height);
    }
}

void Elevator::reloadPID() {
    m_settings->update();

    float p = 0.f;
    float i = 0.f;
    float d = 0.f;

    // Set shooter rotator PID
    p = m_settings->getFloat("PID_ELEVATOR_P");
    i = m_settings->getFloat("PID_ELEVATOR_I");
    d = m_settings->getFloat("PID_ELEVATOR_D");
    m_liftGrbx->setPID(p, i, d);
}

