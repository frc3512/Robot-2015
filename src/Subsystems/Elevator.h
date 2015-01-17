/*
 * Elevator.h
 *
 *  Created on: Jan 16, 2015
 *      Author: acf
 */

#ifndef SRC_ELEVATOR_H_
#define SRC_ELEVATOR_H_

#include "WPILib.h"

class Elevator {
public:
	enum IntakeMotorState {
		S_STOPPED,
		S_FORWARD,
		S_REVERSED
	};

	Elevator();
	virtual ~Elevator();

	// Grabs tote with elevator
	void elevatorGrab ( bool state );
	void intakeGrab ( bool state );
	void intakeVer ( bool state);
	void intakeWheels ( IntakeMotorState state );


private:
	Solenoid *m_grabSolenoid;

	// Intake
	Solenoid *m_intakeVertical;
	Solenoid *m_intakeGrabber;
	CANTalon *m_intakeWheels;
};

#endif /* SRC_ELEVATOR_H_ */
