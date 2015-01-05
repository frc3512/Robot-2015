// =============================================================================
// File Name: GearBox.hpp
// Description: Represents a gear box with up to 3 motors and an encoder
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#ifndef GEARBOX_HPP
#define GEARBOX_HPP

#include <vector>
#include <PIDOutput.h>
#include <PIDSource.h>
#include <iostream>

class Encoder;
class PIDController;
class Solenoid;
class SpeedController;

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
    GearBox( unsigned int shifterChan , unsigned int encA , unsigned int encB ,
             unsigned int motor1 , unsigned int motor2 = 0 ,
             unsigned int motor3 = 0 );
    virtual ~GearBox();

    // Enables PID controller automatically and sets its setpoint
    void setSetpoint( float setpoint );

    // Returns setpoint of PID controller
    float getSetpoint() const;

    // Disables PID controller and sets the motor speeds manually
    void setManual( float value );

    float getManual() const;

    // Set P, I, and D terms for PID controller
    void setPID( float p , float i , float d );

    // Set feed-forward term on PID controller
    void setF( float f );

    // Calls Encoder::SetDistancePerPulse internally
    void setDistancePerPulse( double distancePerPulse );

    // Determines whether encoder returns distance or rate from PIDGet()
    void setPIDSourceParameter( PIDSource::PIDSourceParameter pidSource );

    // Resets encoder distance to 0
    void resetEncoder();

    // Calls respective functions of Encoder class internally
    double getDistance() const;
    double getRate() const;

    // Reverses gearbox drive direction
    void setReversed( bool reverse );

    // Returns motor reversal state of gearbox
    bool isReversed() const;

    // Shifts gearbox to another gear if available
    void setGear( bool gear );

    // Gets current gearbox gear if available (false if not)
    bool getGear() const;

    // Sets motor speed to 'output'
    void PIDWrite( float output );

    // updates the gear to targetState if it is safe todo so.
    void updateGear();

    bool onTarget();

    void resetPID();

    PIDController* m_pid;

private:
    Encoder* m_encoder;

    Solenoid* m_shifter;

    bool m_isReversed;
    bool m_havePID;
    bool m_targetGear;

    std::vector<SpeedController*> m_motors;
};

#include "GearBox.inl"

#endif // GEARBOX_HPP

