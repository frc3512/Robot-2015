#include "Claw.hpp"

#ifndef M_PI
#define M_PI 3.14159265
#endif

#include <Solenoid.h>
#include <DriverStation.h>

Claw::Claw( unsigned int clawRotatePort ,
            unsigned int clawWheelPort ,
            unsigned int zeroSwitchPort ,
            unsigned int haveBallPort ) : m_settings( "RobotSettings.txt" ) {
    m_clawRotator = new GearBox<Talon>( 0 , 7 , 8 , clawRotatePort );
    m_intakeWheel = new GearBox<Talon>( 0 , 0 , 0 , clawWheelPort );

    // Sets degrees rotated per pulse of encoder
    m_clawRotator->setDistancePerPulse( ( 1.0 / 71.0f ) * 14.0 / 44.0 );
    m_clawRotator->setReversed( true );

    m_ballShooter.push_back( new Solenoid( 8 ) );
    m_ballShooter.push_back( new Solenoid( 2 ) );
    m_ballShooter.push_back( new Solenoid( 3 ) );
    m_ballShooter.push_back( new Solenoid( 6 ) );

    m_zeroSwitch = new DigitalInput( zeroSwitchPort );
    m_haveBallSwitch = new DigitalInput( haveBallPort );

    // Set up interrupt for encoder reset
    m_zeroSwitch->RequestInterrupts( Claw::ResetClawEncoder , this );
    m_zeroSwitch->SetUpSourceEdge( true , true );
    // m_zeroSwitch->EnableInterrupts();

    // Set up interrupt for catching ball
    m_haveBallSwitch->RequestInterrupts( Claw::CloseClaw , this );
    m_haveBallSwitch->SetUpSourceEdge( false , true );
    m_haveBallSwitch->EnableInterrupts();

    // magical values found using empirical testing don't change.
    setK( 0.238f );
    m_l = 69.0f;

    m_collectorArm = new Solenoid( 5 );
    m_vacuum = new Relay( 2 , Relay::kForwardOnly );

    m_lastZeroSwitch = true;

    ReloadPID();
    m_shooterStates = SHOOTER_IDLE;
}

Claw::~Claw() {
    // Free solenoids
    for ( unsigned int i = 0 ; i < m_ballShooter.size() ; i++ ) {
        delete m_ballShooter[i];
    }

    delete m_collectorArm;

    m_zeroSwitch->DisableInterrupts();
    delete m_zeroSwitch;

    m_haveBallSwitch->DisableInterrupts();
    delete m_haveBallSwitch;

    delete m_vacuum;
    m_ballShooter.clear();
}

void Claw::SetAngle( float shooterAngle ) {
    m_clawRotator->setSetpoint( shooterAngle );
    m_setpoint = shooterAngle;
}

void Claw::ManualSetAngle( float value ) {
    if ( ( !m_zeroSwitch->Get() && value > 0 ) || m_zeroSwitch->Get() ) {
        m_clawRotator->setManual( value );
    }
}
void Claw::testClaw() {
    std::vector<Solenoid*>::iterator it;
    DriverStation* dsInstance = DriverStation::GetInstance();

    it = m_ballShooter.begin();
    while ( it != m_ballShooter.end() ) {
        ( *it )->Set( true );
        Wait( 1.5 );
        ( *it )->Set( false );
        Wait( 1.5 );
        m_vacuum->Set( Relay::kOn );
        Wait( 1.5 );
        m_vacuum->Set( Relay::kOff );
        Wait( 1.5 );
        it++;

        if ( dsInstance->IsDisabled() ) {
            return;
        }
    }
}

double Claw::GetTargetAngle() const {
    return m_clawRotator->getSetpoint();
}

float Claw::GetWheelManual() const {
    return m_clawRotator->getManual();
}

double Claw::GetAngle() {
    return m_clawRotator->getDistance();
}

bool Claw::AtAngle() const {
    return m_clawRotator->onTarget();
}

void Claw::SetWheelSetpoint( float speed ) {
    m_intakeWheel->setSetpoint( speed );
}

void Claw::SetWheelManual( float speed ) {
    m_intakeWheel->setManual( speed );
}

void Claw::ResetEncoders() {
    m_clawRotator->resetEncoder();
    m_intakeWheel->resetEncoder();
}

void Claw::ReloadPID() {
    m_settings.update();

    float p = 0.f;
    float i = 0.f;
    float d = 0.f;

    // Set shooter rotator PID
    p = m_settings.getFloat( "PID_ARM_ROTATE_P" );
    i = m_settings.getFloat( "PID_ARM_ROTATE_I" );
    d = m_settings.getFloat( "PID_ARM_ROTATE_D" );
    m_clawRotator->setPID( p , i , d );
}

void Claw::Shoot() {
    if ( m_shooterStates == SHOOTER_IDLE ) {
        m_collectorArm->Set( true );
        m_shooterStates = SHOOTER_ARMISLIFTING;
        m_shootTimer.Start();
        m_shootTimer.Reset();
    }
}

void Claw::SetCollectorMode( bool collectorMode ) {
    m_collectorArm->Set( collectorMode );
}
bool Claw::GetCollectorMode() {
    return m_collectorArm->Get();
}

void Claw::Update() {
    if ( m_shooterStates == SHOOTER_ARMISLIFTING &&
         m_shootTimer.HasPeriodPassed( 0.5 ) ) {
        for ( unsigned int i = 0 ; i < m_ballShooter.size() ; i++ ) {
            m_ballShooter[i]->Set( true );
        }
        m_shootTimer.Reset();
        m_shooterStates = SHOOTER_SHOOTING;
    }
    if ( m_shooterStates == SHOOTER_SHOOTING &&
         m_shootTimer.HasPeriodPassed( 2.0 ) ) {
        for ( unsigned int i = 0 ; i < m_ballShooter.size() ; i++ ) {
            m_ballShooter[i]->Set( false );
        }
        m_vacuum->Set( Relay::kOn );
        m_shootTimer.Reset();
        m_shooterStates = SHOOTER_VACUUMING;
    }
    if ( m_shooterStates == SHOOTER_VACUUMING &&
         m_shootTimer.HasPeriodPassed( 3.0 ) ) {
        m_vacuum->Set( Relay::kOff );
        m_collectorArm->Set( false );

        m_shootTimer.Reset();
        m_shooterStates = SHOOTER_IDLE;
    }

    setF( calcF() );

    // Spins intake wheel to keep ball in while rotating claw at high speeds
    if ( fabs( m_clawRotator->getRate() ) > 35.f ) {
        SetWheelManual( -1.f );
    }

    /* Fixes arm, when at reset angle, not touching zeroSwitch due to gradual
     * encoder error. If limit switch isn't pressed but arm is supposedly at
     * zeroing point or farther:
     */
    if ( m_zeroSwitch->Get() && GetTargetAngle() <= 1.f &&
         m_clawRotator->onTarget() ) {
        m_clawRotator->setSetpoint( GetTargetAngle() - 5.f );
    }

    // If wasn't pressed last time and is now
    if ( !m_zeroSwitch->Get() ) {
        ResetClawEncoder( 0 , this );
    }

    m_lastZeroSwitch = m_zeroSwitch->Get();
}

void Claw::setF( float f ) {
    m_clawRotator->setF( f );
}

void Claw::setK( float k ) {
    m_k = k;
}

float Claw::calcF() {
    if ( GetTargetAngle() == 0 ) {
        return 0.0f;
    }

    return m_k * cos( ( GetAngle() + m_l ) * M_PI / 180.0f ) / GetTargetAngle();
}
bool Claw::onTarget() {
    return m_clawRotator->onTarget();
}
bool Claw::IsShooting() const {
    if ( m_shooterStates != SHOOTER_IDLE ) {
        return true;
    }
    else {
        return false;
    }
}

void Claw::ResetClawEncoder( unsigned int interruptAssertedMask , void* obj ) {
    Claw* claw = static_cast<Claw*>( obj );

    if ( claw->GetTargetAngle() <= 0.0 ) {
        claw->m_clawRotator->setSetpoint( 0.f );
        claw->m_clawRotator->resetPID();
        claw->m_clawRotator->resetEncoder();
    }
}

void Claw::CloseClaw( unsigned int interruptAssertedMask , void* obj ) {
    std::cout << "CloseClaw INTERRUPT\n";
    Claw* claw = static_cast<Claw*>( obj );

    if ( !claw->IsShooting() ) {
        claw->SetCollectorMode( false );
    }
}

