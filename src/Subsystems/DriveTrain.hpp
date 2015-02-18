// =============================================================================
// File Name: DriveTrain.hpp
// Description: Provides an interface for this year's drive train
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#ifndef DRIVE_TRAIN_HPP
#define DRIVE_TRAIN_HPP

#include "../Settings.hpp"
#include "../MotionProfile/BezierTrapezoidProfile.hpp"
#include "GearBox.hpp"

#include <memory>
#include <CANTalon.h>

class DriveTrain : public BezierTrapezoidProfile {
public:
    DriveTrain();
    virtual ~DriveTrain();

    /* Drives robot with given speed and turn values [-1..1].
     * This is a convenience function for use in Operator Control.
     */
    void drive(float throttle, float turn, bool isQuickTurn = false);

    // Sets joystick deadband
    void setDeadband(float band);

    // Set encoder distances to 0
    void resetEncoders();

    // Reload PID constants
    void reloadPID();

    // Set wheel setpoints (see GearBox::setSetpoint(float))
    void setLeftSetpoint(double setpt);
    void setRightSetpoint(double setpt);

    // Directly set wheel speeds [0..1] (see GearBox::setManual(float))
    void setLeftManual(float value);
    void setRightManual(float value);

    // Returns encoder distances
    double getLeftDist();
    double getRightDist();

    // Returns encoder rates
    double getLeftRate();
    double getRightRate();

    // Returns encoder PID loop setpoints
    double getLeftSetpoint();
    double getRightSetpoint();

    void setControlMode(CANTalon::ControlMode ctrlMode);

    const static float maxWheelSpeed;

private:
    Settings m_settings;

    float m_deadband;
    float m_sensitivity;

    // Cheesy Drive variables
    float m_oldTurn;
    float m_quickStopAccumulator;
    float m_negInertiaAccumulator;

    std::unique_ptr<GearBox<CANTalon>> m_leftGrbx;
    std::unique_ptr<GearBox<CANTalon>> m_rightGrbx;

    /* Zeroes value if its inside deadband range, and rescales values outside
     * of it
     */
    float applyDeadband(float value);

    // Limits 'value' to within +- 'limit' (limit should be positive)
    template <class T>
    T limit(T value, T limit);
};

#include "DriveTrain.inl"

#endif // DRIVE_TRAIN_HPP

