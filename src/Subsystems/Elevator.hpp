// =============================================================================
// File Name: Elevator.hpp
// Description: Provides an interface for the robot's elevator
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#ifndef ELEVATOR_HPP
#define ELEVATOR_HPP

class Solenoid;
class DigitalInput;
class CANTalon;

#include "../MotionProfile/TrapezoidProfile.hpp"
#include <vector>
#include <memory>
#include <thread>
#include <atomic>
#include <Timer.h>
#include <string>

#include "GearBox.hpp"
#include "../Settings.hpp"

class Elevator : public TrapezoidProfile {
public:
    enum ElevatorState {
        STATE_IDLE,
        STATE_WAIT_INITIAL_HEIGHT,
        STATE_SEEK_DROP_TOTES,
        STATE_RELEASE,
        STATE_SEEK_GROUND,
        STATE_GRAB,
        STATE_SEEK_HALF_TOTE,
        STATE_INTAKE_IN
    };

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

    // Sets speed of lift gearbox directly if manual input is enabled
    void setManualLiftSpeed(double value);

    void setManualMode(bool on);
    bool isManualMode();

    // Sets setpoint for elevator PID controller
    void setHeight(double height);

    // Gets the current height as integrated from the encoder
    double getHeight();

    void reloadPID();

    bool onTarget();

    void resetEncoder();
    void pollLimitSwitches();

    // Takes a string representing the name of the height
    void raiseElevator(std::string level);
    void setProfileHeight(double height);
    double getLevelHeight(std::string level) const;

    void stackTotes();
    void updateState();
    std::string to_string(ElevatorState state);

    void manualChangeSetpoint(double delta);

protected:
    std::unique_ptr<Settings> m_settings;

private:
    void stateChanged(ElevatorState oldState, ElevatorState newState);

    std::unique_ptr<Solenoid> m_grabSolenoid;
    std::unique_ptr<DigitalInput> m_bottomLimit;

    // Intake
    IntakeMotorState m_intakeState;
    std::unique_ptr<Solenoid> m_intakeVertical;
    std::unique_ptr<Solenoid> m_intakeGrabber;
    std::unique_ptr<CANTalon> m_intakeWheelLeft;
    std::unique_ptr<CANTalon> m_intakeWheelRight;
    std::unique_ptr<DigitalInput> m_frontLeftLimit;
    std::unique_ptr<DigitalInput> m_frontRightLimit;
    std::unique_ptr<GearBox<CANTalon>> m_liftGrbx;
    bool m_manual;

    std::map<std::string, double> m_toteHeights;
    std::unique_ptr<Timer> m_profileTimer;
    std::atomic<bool> m_updateProfile;
    std::thread* m_profileUpdater;

    ElevatorState m_state;
    std::unique_ptr<Timer> m_grabTimer;

    double m_maxHeight;

    static void resetEncoder(uint32_t interruptAssertedMask, void* param);
};

#endif // ELEVATOR_HPP

