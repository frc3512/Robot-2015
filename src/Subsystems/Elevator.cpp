/*
 * Elevator.cpp
 *
 *  Created on: Jan 16, 2015
 *      Author: acf
 */

#include "Elevator.hpp"

Elevator::Elevator() {
    m_grabSolenoid = new Solenoid(1);
    m_intakeVertical = new Solenoid(2);
    m_intakeGrabber = new Solenoid(3);
    m_intakeWheels = new CANTalon(6);
    m_settings = new Settings("/home/lvuser/RobotSettings.txt");
    m_intakeState = S_STOPPED;
    m_manual = false;

    m_liftmotors = new GearBox<CANTalon> (-1, 4, 5);
    m_liftmotors->setDistancePerPulse(360);
    reloadPID();

}

Elevator::~Elevator() {
	delete m_grabSolenoid;
	delete m_intakeVertical;
	delete m_intakeGrabber;
	delete m_intakeWheels;
	delete m_settings;
	delete m_liftmotors;
}

void Elevator::reloadPID() {
    m_settings->update();

    float p = 0.f;
    float i = 0.f;
    float d = 0.f;

    // Set shooter rotator PID
    p = m_settings->getFloat( "PID_ELEVATOR_P" );
    i = m_settings->getFloat( "PID_ELEVATOR_I" );
    d = m_settings->getFloat( "PID_ELEVATOR_D" );
    m_liftmotors->setPID( p , i , d );
}

void Elevator::elevatorGrab(bool state) {
    m_grabSolenoid->Set(state);
}

void Elevator::intakeGrab(bool state) {
    m_intakeGrabber->Set(state);
}

void Elevator::intakeVer(bool state) {
    m_intakeVertical->Set(state);
}

bool Elevator::getElevatorGrab() {
    return m_grabSolenoid->Get();
}

bool Elevator::getIntakeGrab() {
    return m_intakeGrabber->Get();
}

bool Elevator::getIntakeVer() {
    return m_intakeVertical->Get();
}

void Elevator::intakeWheels(IntakeMotorState state) {
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

Elevator::IntakeMotorState Elevator::getIntakeWheels() {
    return m_intakeState;
}

void Elevator::setIntakeMotorState(float value) {
	if(m_manual == true) {
		m_liftmotors->setManual(value);
	}

}

void Elevator::setManualMode(bool on) {
	m_manual = on;
}

bool Elevator::getManualMode() {
	return m_manual;
}

void Elevator::setHeight(float height) {
	if(m_manual == false) {
		m_liftmotors->setSetpoint(height);
	}

}
