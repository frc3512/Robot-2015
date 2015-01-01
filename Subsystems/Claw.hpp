#ifndef CLAW_HPP
#define CLAW_HPP

#include <vector>

#include <Encoder.h>
#include <Talon.h>
#include <Timer.h>
#include <DigitalInput.h>
#include <Relay.h>

#include "../Settings.hpp"
#include "GearBox.hpp"

class Solenoid;

class Claw {
public:
    Claw (unsigned int clawRotatePort, unsigned int clawWheelPort,
            unsigned int zeroSwitchPort, unsigned int haveBallPort);
    ~Claw();

    // Set mode of collector
    void SetCollectorMode( bool collectorMode );

    bool GetCollectorMode();

    // Set angle of claw in degrees
    void SetAngle( float shooterAngle );

    // Manually set the value of the angle motor
    void ManualSetAngle(float value);

	// Returns setpoint of rotator's internal PID loop in degrees
    double GetTargetAngle() const;

    // Sets speed of claw's intake wheel
    void SetWheelSetpoint( float speed );
    void SetWheelManual( float speed );

    float GetWheelManual() const;

    double GetAngle();

    bool AtAngle() const;

    // Set encoder distances to 0
    void ResetEncoders();

    // Reload PID constants
    void ReloadPID();

    // StartShoots activating solenoids to shoot ball
    void Shoot();

    // Continues process of shooting
    void Update();

    // Returns true if this instance is in the middle of shooting
    bool IsShooting() const;

    bool onTarget();

    void setK(float k);

    void setF(float f);

    void testClaw();

    float calcF();
    typedef enum ShooterStates {
    	SHOOTER_IDLE,
    	SHOOTER_SHOOTING,
    	SHOOTER_VACUUMING,
    	SHOOTER_ARMISLIFTING
    } ShooterStates;

private:
    Settings m_settings;
    float m_k;
    float m_l;

    GearBox<Talon>* m_clawRotator;
    GearBox<Talon>* m_intakeWheel;

    // Resets the encoder in m_clawRotator to 0
    DigitalInput* m_zeroSwitch;

    // Returns true when ball is hitting limit switch in claw
    DigitalInput* m_haveBallSwitch;

    Timer m_shootTimer;
    ShooterStates m_shooterStates;

    float m_setpoint;

    std::vector<Solenoid*> m_ballShooter;
    Relay *m_vacuum;
    Solenoid *m_collectorArm;

    bool m_lastZeroSwitch;

    /* Used for claw rotation encoder interrupt
     * 'void* obj' should be a pointer to an instance of this class
     */
    static void ResetClawEncoder( unsigned int interruptAssertedMask, void* obj );

    static void CloseClaw( unsigned int interruptAssertedMask, void* obj );
};

#endif // CLAW_HPP
