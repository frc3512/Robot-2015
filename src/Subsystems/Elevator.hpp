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
    void elevatorGrab(bool state);
    bool getElevatorGrab();
    void intakeGrab(bool state);
    bool getIntakeGrab();
    void intakeVer(bool state);
    bool getIntakeVer();
    void stopped(bool state);
    void intakeWheels(IntakeMotorState state);
    IntakeMotorState getIntakeWheels();
    void setIntakeMotorState(float value);


private:
    Solenoid* m_grabSolenoid;

    // Intake
    IntakeMotorState m_intakeState;
    Solenoid* m_intakeVertical;
    Solenoid* m_intakeGrabber;
    CANTalon* m_intakeWheels;
    CANTalon* m_liftmotor_0;
    CANTalon* m_liftmotor_1;
};

#endif /* SRC_ELEVATOR_H_ */

