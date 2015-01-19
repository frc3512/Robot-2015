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
	m_intakeWheels = new CANTalon(4);
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

void Elevator::intakeWheels(IntakeMotorState state) {

	if(state == S_STOPPED) {
		m_intakeWheels->Set(0);
	} else if (state == S_FORWARD) {
		m_intakeWheels->Set(1);
	} else if (state == S_REVERSED) {
		m_intakeWheels->Set(-1);
	}

}
