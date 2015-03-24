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

#include "SubsystemBase.hpp"
#include "../MotionProfile/TrapezoidProfile.hpp"
#include <vector>
#include <memory>
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

    // Sets direction of intake wheels
    void setIntakeDirectionLeft(IntakeMotorState state);
    void setIntakeDirectionRight(IntakeMotorState state);
    IntakeMotorState getIntakeDirection() const;

    // Sets speed of lift gearbox directly if manual input is enabled
    void setManualLiftSpeed(double value);
    double getManualLiftSpeed() const;

    void setManualMode(bool on);
    bool isManualMode() const;

    /* If feeding is enabled, auto stacking levels will be shifted up by one
     * tote to accommodate the feeder station.
     */
    void setFeeding(bool feed);
    bool isFeeding() const;

    // Sets setpoint for elevator PID controller
    void setHeight(double height);
    double getHeight() const;

    void reloadPID();
    void resetEncoders();

    // Returns true if both of front limit switches are contacted
    bool pollFrontLimitSwitches() const;

    // Periodic
    void pollLiftLimitSwitches();

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
    bool m_feeding;

    std::map<std::string, double> m_toteHeights;
    std::unique_ptr<Timer> m_profileTimer;
    std::atomic<bool> m_updateProfile;
    std::thread* m_profileUpdater;

    StateMachine m_autoStackSM;
    std::unique_ptr<Timer> m_grabTimer;
    bool m_startAutoStacking;

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

