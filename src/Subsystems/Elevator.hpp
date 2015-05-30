// =============================================================================
// File Name: Elevator.hpp
// Description: Provides an interface for the robot's elevator
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#ifndef ELEVATOR_HPP
#define ELEVATOR_HPP

class Solenoid;
class CANTalon;

#include "SubsystemBase.hpp"
#include "../MotionProfile/TrapezoidProfile.hpp"
#include <vector>
#include <thread>
#include <atomic>
#include <Timer.h>
#include <string>

#include "GearBox.hpp"
#include "../StateMachine.hpp"

class Elevator : public SubsystemBase, public TrapezoidProfile {
public:
    enum IntakeMotorState {
        S_STOPPED,
        S_FORWARD,
        S_REVERSE,
        S_ROTATE_CCW,
        S_ROTATE_CW
    };

    Elevator();
    ~Elevator();

    // Actuates elevator tines in/out
    void elevatorGrab(bool state);
    bool isElevatorGrabbed() const;

    // Actuates intake arms in/out
    void intakeGrab(bool state);
    bool isIntakeGrabbed() const;

    // Stows/unstows intake arms
    void stowIntake(bool state);
    bool isIntakeStowed() const;

    // Actuates container graba in/out
    void containerGrab(bool state);
    bool isContainerGrabbed() const;

    // Sets direction of intake wheels
    void setIntakeDirectionLeft(IntakeMotorState state);
    void setIntakeDirectionRight(IntakeMotorState state);
    IntakeMotorState getIntakeDirection() const;

    // Sets speed of lift gearbox directly if manual input is enabled
    void setManualLiftSpeed(double value);
    double getManualLiftSpeed() const;

    void setManualMode(bool on);
    bool isManualMode() const;

    // Sets setpoint for elevator PID controller
    void setHeight(double height);
    double getHeight() const;

    void reloadPID();
    void resetEncoders();

    // Takes a string representing the name of the height
    void raiseElevator(std::string level);
    void setProfileHeight(double height);
    double getLevelHeight(std::string level) const;

    void manualChangeSetpoint(double delta);

    void stackTotes();
    bool isStacking() const;
    void cancelStack();

    // Periodic
    void updateState();

private:
    Solenoid m_elevatorGrabber{3};
    Solenoid m_containerGrabber{4};
    GearBox<CANTalon> m_liftGrbx{-1, 7, 2};
    bool m_manual{false};

    // Intake
    IntakeMotorState m_intakeState;
    Solenoid m_intakeStower{1};
    Solenoid m_intakeGrabber{2};
    CANTalon m_intakeWheelLeft{3};
    CANTalon m_intakeWheelRight{4};

    std::map<std::string, double> m_toteHeights;
    Timer m_profileTimer;
    std::atomic<bool> m_updateProfile{true};
    std::thread* m_profileUpdater;

    StateMachine m_autoStackSM;
    Timer m_grabTimer;
    bool m_startAutoStacking{false};
    bool m_wasAtGround{false};

    double m_maxHeight;

    /* Maximum velocity and time to maximum velocity constants to load from the
     * config file
     */
    double m_maxv_a;
    double m_ttmaxv_a;
    double m_maxv_b;
    double m_ttmaxv_b;
};

#endif // ELEVATOR_HPP

