// =============================================================================
// File Name: AutonMotionProfile.cpp
// Description: Drives set distance with motion profiles
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "../Robot.hpp"
#include <Timer.h>

bool Robot::checkReflectiveStrips() {
    return true;
}

void Robot::RightLeftAuton() {
    float targetValue;
    const float targetDistance = 295.0;
    Timer* timer = new Timer;
    timer->Reset();
    robotDrive->resetEncoders();
    timer->Start();
    bool state = checkReflectiveStrips();

    robotDrive->drive( 0 , 0 );
    Wait( 0.5 );
    robotDrive->drive( -0.1 , 0 );
    Wait( .25 );
    claw->SetAngle( 115.0 );
    Wait( 0.5 );

    /* "&& robotDrive->getRightDist() > -5.0" ensures robot doesn't drive
     * backwards
     */
    while ( robotDrive->getRightDist() < targetDistance && IsEnabled() &&
            robotDrive->getRightDist() > -5.0 && IsAutonomous() ) {
        targetValue = -0.6 *
                      ( 1.f - robotDrive->getRightDist() / targetDistance );
        robotDrive->drive( targetValue , 0 );

        // std::cout << "right distance: " << (robotDrive->getRightDist())<< std::endl;
        // std::cout << "left distance: " << (robotDrive->getLeftDist()) << std::endl;
        // std::cout << "targetValue: " << targetValue << std::endl;

        if ( claw->onTarget() ) {
            claw->SetWheelManual( 0.0 );
        }
        claw->Update();
        Wait( 0.1 );
    }

    robotDrive->drive( 0 , 0 );
    // std::cout << "final right distance: " << (robotDrive->getRightDist()) << std::endl;
    // std::cout << "final left distance: " << (robotDrive->getLeftDist()) << std::endl;

    while ( robotDrive->getLeftDist() * -1 < robotDrive->getRightDist() &&
            IsEnabled() && IsAutonomous() ) {
        robotDrive->setLeftManual( 0.3 );
    }
    claw->SetWheelManual( 0.0 );
    robotDrive->drive( -0.1 , 0 );
    Wait( 0.1 );
    robotDrive->drive( 0 , 0 );
    if ( state == true ) {
        claw->Shoot();
    }
    else {
        while ( timer->HasPeriodPassed( 5 ) == false ) {
            Wait( 0.01 );
        }
        claw->Shoot();
    }
    while ( IsEnabled() && IsAutonomous() ) {
        claw->Update();
        Wait( 0.1 );
    }
}

