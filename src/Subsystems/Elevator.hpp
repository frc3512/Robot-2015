/*
 * Elevator.h
 *
 *  Created on: Jan 16, 2015
 *      Author: acf
 */

#ifndef SRC_ELEVATOR_H_
#define SRC_ELEVATOR_H_

class Solenoid;
class CANTalon;

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

    // Actuates elevator tines in/out
    void elevatorGrab(bool state);
    bool getElevatorGrab();

    // Actuates intake arms in/out
    void intakeGrab(bool state);
    bool getIntakeGrab();

    // Stows/unstows intake arms
    void stowIntake(bool state);
    bool isIntakeStowed();

    // Sets direction of intake wheels
    void setIntakeDirection(IntakeMotorState state);
    IntakeMotorState getIntakeDirection();

    void stop(bool state);

    // Sets speed of lift gearbox directly if manual input is enabled
    void setManualLiftSpeed(float value);

    void setManualMode(bool on);
    bool getManualMode();

    // Sets setpoint for elevator PID controller
    void setHeight(float height);

    void reloadPID();

    bool onTarget();

private:
    Solenoid* m_grabSolenoid;

    // Intake
    IntakeMotorState m_intakeState;
    Solenoid* m_intakeVertical;
    Solenoid* m_intakeGrabber;
    CANTalon* m_intakeWheels;
    GearBox<CANTalon>* m_liftGrbx;
    Settings* m_settings;
    bool m_manual;
};

#endif /* SRC_ELEVATOR_H_ */

