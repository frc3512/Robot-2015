// =============================================================================
// File Name: GearBox.hpp
// Description: Represents a gear box with up to 3 motors and an encoder
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#ifndef GEARBOX_HPP
#define GEARBOX_HPP

#include <vector>
#include <memory>
#include <PIDOutput.h>
#include <PIDSource.h>
#include <CANTalon.h>

class Encoder;
class PIDController;
class Solenoid;

/* Notes:
 * The template type of this template class is only used for creating the right
 * type of motor controllers for the gearbox.
 *
 * It is assumed that only one type of motor controller is used per gearbox.
 *
 * Up to three motors can be specified per gearbox, since drive train gearboxes
 * will use up to three and other gearboxes will use less.
 */

template <class T>
class GearBox : public PIDOutput {
public:
    GearBox(int shifterChan, int encA, int encB,
            int motor1, int motor2 = -1,
            int motor3 = -1);
    virtual ~GearBox();

    enum PIDMode {
        Position,
        Speed,
        Raw // Returns voltage [0..1] when used
    };

    // Enables PID controller automatically and sets its setpoint
    void setSetpoint(float setpoint);

    float getSetpoint();

    // Disables PID controller and sets the motor speeds manually
    void setManual(float value);

    // Returns current speed/position/voltage setting of motor controller(s)
    float get(PIDMode mode = PIDMode::Raw) const;

    // Set P, I, and D terms for PID controller
    void setPID(float p, float i, float d);

    // Set feed-forward term on PID controller
    void setF(float f);

    // Calls Encoder::SetDistancePerPulse internally
    void setDistancePerPulse(double distancePerPulse);

    // Determines whether encoder returns distance or rate from PIDGet()
    void setPIDSourceParameter(PIDSource::PIDSourceParameter pidSource);

    // Resets encoder distance to 0
    void resetEncoder();

    // Reverses gearbox drive direction
    void setMotorReversed(bool reverse);

    // Returns motor reversal state of gearbox
    bool isMotorReversed() const;

    // Reverses gearbox drive direction
    void setEncoderReversed(bool reverse);

    // Returns motor reversal state of gearbox
    bool isEncoderReversed() const;

    // Shifts gearbox to another gear if available
    void setGear(bool gear);

    // Gets current gearbox gear if available (false if not)
    bool getGear() const;

    bool onTarget();

    void resetPID();

private:
    // Sets motor speed to 'output'
    void PIDWrite(float output);

    PIDController* m_pid;
    Encoder* m_encoder;
    Solenoid* m_shifter;

    double m_distancePerPulse;
    bool m_isMotorReversed;
    bool m_isEncoderReversed;
    bool m_havePID;

    std::vector<std::unique_ptr<T>> m_motors;
};

#include "GearBox.inl"

#if 0

template <>
class GearBox<CANTalon> {
public:
    GearBox(int shifterChan, int motor1, int motor2 = -1, int motor3 = -1);

    enum PIDMode {
        Position,
        Speed,
        Raw // Returns voltage [0..1] when used
    };

    // Enables PID controller automatically and sets its setpoint
    void setSetpoint(float setpoint);

    float getSetpoint();

    // Disables PID controller and sets the motor speeds manually
    void setManual(float value);

    // Returns current speed/position/voltage setting of motor controller(s)
    float get(PIDMode mode = PIDMode::Raw);

    // Set P, I, and D terms for PID controller
    void setPID(float p, float i, float d);

    // Set feed-forward term on PID controller
    void setF(float f);

    void setDistancePerPulse(double distancePerPulse);

    // Determines whether encoder returns distance or rate from PIDGet()
    void setControlMode(CANTalon::ControlMode ctrlMode =
                            CANTalon::kPercentVbus);

    // Resets encoder distance to 0
    void resetEncoder();

    // Reverses gearbox drive direction
    void setMotorReversed(bool reverse);

    // Returns motor reversal state of gearbox
    bool isMotorReversed() const;

    // Reverses gearbox drive direction
    void setEncoderReversed(bool reverse);

    // Returns motor reversal state of gearbox
    bool isEncoderReversed() const;

    // Shifts gearbox to another gear if available
    void setGear(bool gear);

    // Gets current gearbox gear if available (false if not)
    bool getGear() const;

    bool onTarget();

    void resetPID();

private:
    std::unique_ptr<Solenoid> m_shifter;

    bool m_isMotorReversed;
    bool m_isEncoderReversed;

    // Conversion factor for setpoints with respect to encoder readings
    double m_distancePerPulse;

    // Store the setpoint, because CANTalon won't give it to us
    float m_setpoint;

    std::vector<std::unique_ptr<CANTalon>> m_motors;
};

#include "GearBoxCANTalon.inl"

#endif

#endif // GEARBOX_HPP

