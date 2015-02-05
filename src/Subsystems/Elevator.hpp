/*
 * Elevator.h
 *
 *  Created on: Jan 16, 2015
 *      Author: acf
 */

#ifndef SRC_ELEVATOR_H_
#define SRC_ELEVATOR_H_

#include "WPILib.h"
#include "GearBox.hpp"
#include "../Settings.hpp"

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
    void reloadPID();
    void setManualMode(bool on);
    bool getManualMode();
    void setHeight(float height);


private:
    Solenoid* m_grabSolenoid;

    // Intake
    IntakeMotorState m_intakeState;
    Solenoid* m_intakeVertical;
    Solenoid* m_intakeGrabber;
    CANTalon* m_intakeWheels;
    GearBox<CANTalon>* m_liftmotors;
    Settings* m_settings;
    bool m_manual;
};

#endif /* SRC_ELEVATOR_H_ */

