// =============================================================================
// File Name: Elevator.hpp
// Description: Provides an interface for the robot's elevator
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#ifndef ELEVATOR_HPP
#define ELEVATOR_HPP

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

    // Get setpoint from PID controller
    float getHeight();

    void reloadPID();

    bool onTarget();


private:
    std::unique_ptr<Solenoid> m_grabSolenoid;

    // Intake
    IntakeMotorState m_intakeState;
    std::unique_ptr<Solenoid> m_intakeVertical;
    std::unique_ptr<Solenoid> m_intakeGrabber;
    std::unique_ptr<CANTalon> m_intakeWheelLeft;
    std::unique_ptr<CANTalon> m_intakeWheelRight;
    std::unique_ptr<GearBox<CANTalon>> m_liftGrbx;
    std::unique_ptr<Settings> m_settings;
    bool m_manual;
};

#endif // ELEVATOR_HPP

