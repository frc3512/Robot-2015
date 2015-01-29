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

	m_liftmotor_0 = new CANTalon(4);
	m_liftmotor_1 = new CANTalon(5);
}

Elevator::~Elevator() {
	// TODO Auto-generated destructor stub
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

	if(state == S_STOPPED) {
		m_intakeWheels->Set(0);
	} else if (state == S_FORWARD) {
		m_intakeWheels->Set(1);
	} else if (state == S_REVERSED) {
		m_intakeWheels->Set(-1);
	}

}

Elevator::IntakeMotorState Elevator::getIntakeWheels() {
	return m_intakeState;
}

void Elevator::setIntakeMotorState(float value) {
	m_liftmotor_0->Set(value);
	m_liftmotor_1->Set(value);
}
