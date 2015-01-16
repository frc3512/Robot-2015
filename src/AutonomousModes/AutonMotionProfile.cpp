// =============================================================================
// File Name: AutonMotionProfile.cpp
// Description: Drives set distance with motion profiles
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "../Robot.hpp"

void Robot::AutonMotionProfile() {
    // Reload drive PID constants
    settings.update();
    robotDrive->reloadPID();

    robotDrive->resetEncoders();

    // Move robot 360 cm forward
    robotDrive->resetTime();
    robotDrive->setGoal( 60 , robotDrive->getLeftDist() , autonTimer->Get() );
    double setpoint = 0.0;
    while ( IsAutonomous() && IsEnabled() && !robotDrive->atGoal() ) {
        DS_PrintOut();

        setpoint =
            robotDrive->updateSetpoint( setpoint , 0 , autonTimer->Get() );
        robotDrive->setLeftSetpoint( setpoint );
        robotDrive->setRightSetpoint( setpoint );

        Wait( 0.01 );
    }

    // Stop moving
    robotDrive->setLeftManual( 0.f );
    robotDrive->setRightManual( 0.f );

}

