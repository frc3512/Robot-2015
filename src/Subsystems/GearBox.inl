// =============================================================================
// File Name: GearBox.inl
// Description: Represents a gear box with up to 3 motors and an encoder
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include <cmath>
#include <Encoder.h>
#include <Solenoid.h>
#include <SpeedController.h>

#include <PIDController.h>

template <class T>
GearBox<T>::GearBox( int shifterChan ,
                     int encA ,
                     int encB ,
                     int motor1 ,
                     int motor2 ,
                     int motor3 ) {
    if ( encA != -1 && encB != -1 ) {
        m_encoder = new Encoder( encA , encB );
        m_pid = new PIDController( 0 , 0 , 0 , 0 , m_encoder , this );

        m_havePID = true;
    }
    else {
        m_encoder = nullptr;
        m_pid = nullptr;

        m_havePID = false;
    }

    if ( shifterChan != -1 ) {
        m_shifter = new Solenoid( shifterChan );
    }
    else {
        m_shifter = nullptr;
    }

    m_isReversed = false;

    m_targetGear = false;

    // Create motor controllers of specified template type
    if ( motor1 != -1 ) {
        m_motors.push_back( new T( motor1 ) );
    }
    if ( motor2 != -1 ) {
        m_motors.push_back( new T( motor2 ) );
    }
    if ( motor3 != -1 ) {
        m_motors.push_back( new T( motor3 ) );
    }
    if ( m_havePID ) {
        m_encoder->SetPIDSourceParameter( Encoder::kDistance );

        // m_pid->SetPercentTolerance( 5.f );
        m_pid->SetAbsoluteTolerance( 1 );

        m_pid->Enable();
    }
}

template <class T>
GearBox<T>::~GearBox() {
    if ( m_havePID ) {
        delete m_pid;
        delete m_encoder;
    }

    if ( m_shifter != nullptr ) {
        delete m_shifter;
    }

    // Free motors
    for ( unsigned int i = 0 ; i < m_motors.size() ; i++ ) {
        delete m_motors[i];
    }
    m_motors.clear();
}

template <class T>
void GearBox<T>::setSetpoint( float setpoint ) {
    if ( m_havePID ) {
        if ( !m_pid->IsEnabled() ) {
            m_pid->Enable();
        }

        m_pid->SetSetpoint( setpoint );
    }
    else {
        // TODO emit warning since PID doesn't work (possibly through logger?)
    }
}

template <class T>
float GearBox<T>::getSetpoint() const {
    if ( m_havePID ) {
        return m_pid->GetSetpoint();
    }
    else {
        // TODO emit warning since PID doesn't work (possibly through logger?)
        return 0.f;
    }
}

template <class T>
void GearBox<T>::setManual( float value ) {
    if ( m_havePID ) {
        if ( m_pid->IsEnabled() ) {
            m_pid->Disable();
        }
    }

    PIDWrite( value );
}

template <class T>
float GearBox<T>::getManual() const {
    if ( !m_isReversed ) {
        return m_motors[0]->Get();
    }
    else {
        return -m_motors[0]->Get();
    }
}

template <class T>
void GearBox<T>::setPID( float p , float i , float d ) {
    if ( m_havePID ) {
        m_pid->SetPID( p , i , d );
    }
    else {
        // TODO emit warning since PID doesn't work (possibly through logger?)
    }
}

template <class T>
void GearBox<T>::setF( float f ) {
    if ( m_havePID ) {
        m_pid->SetPID( m_pid->GetP() , m_pid->GetI() , m_pid->GetD() , f );
    }
    else {
        // TODO emit warning since PID doesn't work (possibly through logger?)
    }
}

template <class T>
void GearBox<T>::setDistancePerPulse( double distancePerPulse ) {
    if ( m_havePID ) {
        m_encoder->SetDistancePerPulse( distancePerPulse );
    }
}

template <class T>
void GearBox<T>::setPIDSourceParameter( PIDSource::PIDSourceParameter pidSource )
{
    if ( m_havePID ) {
        m_encoder->SetPIDSourceParameter( pidSource );
    }
}

template <class T>
void GearBox<T>::resetEncoder() {
    if ( m_havePID ) {
        m_encoder->Reset();
    }
    else {
        // TODO emit warning since PID doesn't work (possibly through logger?)
    }
}

template <class T>
double GearBox<T>::getDistance() const {
    if ( m_havePID ) {
        return m_encoder->GetDistance();
    }
    else {
        // TODO emit warning since PID doesn't work (possibly through logger?)
        return 0.f;
    }
}

template <class T>
double GearBox<T>::getRate() const {
    if ( m_havePID ) {
        return m_encoder->GetRate();
    }
    else {
        // TODO emit warning since PID doesn't work (possibly through logger?)
        return 0.f;
    }
}

template <class T>
void GearBox<T>::setReversed( bool reverse ) {
    m_isReversed = reverse;
}

template <class T>
bool GearBox<T>::isReversed() const {
    return m_isReversed;
}

template <class T>
void GearBox<T>::setGear( bool gear ) {
    if ( m_shifter != nullptr ) {
        m_targetGear = gear;
    }
}

template <class T>
bool GearBox<T>::getGear() const {
    if ( m_shifter != nullptr ) {
        return m_shifter->Get();
    }
    else {
        return false;
    }
}

template <class T>
void GearBox<T>::PIDWrite( float output ) {
    for ( unsigned int i = 0 ; i < m_motors.size() ; i++ ) {
        if ( !m_isReversed ) {
            m_motors[i]->Set( output );
        }
        else {
            m_motors[i]->Set( -output );
        }
    }

    updateGear();
}

template <class T>
void GearBox<T>::updateGear() {
    if ( m_shifter == nullptr || m_targetGear == m_shifter->Get() ) {
        return;
    }

    for ( unsigned int i = 0 ; i < m_motors.size() ; i++ ) {
        if ( fabs( m_motors[i]->Get() ) < 0.12 ) {
            return;
        }
    }

    // if ( (m_pid->IsEnabled() && fabs(m_encoder->GetRate()) > 50) || !m_pid->IsEnabled()) {
    m_shifter->Set( m_targetGear );

    // }
}

template <class T>
bool GearBox<T>::onTarget() {
#if 0
    if ( !m_havePID ) {
        return false;
    }

    return fabs( m_pid->GetError() / 100.f ) <= m_pid->GetTolerance() && fabs(
        m_pid->GetDeltaError() ) <= m_pid->GetTolerance();
#endif
    return m_pid->OnTarget();
}

template <class T>
void GearBox<T>::resetPID() {
    m_pid->Reset();
    m_pid->Enable();
}

