#include "../Robot.hpp"
#include <Timer.h>

void Robot::DriveForwardAuton() {
    robotDrive->drive( 0 , 0 );
    Wait( 0.5 );
    robotDrive->drive( -0.1 , 0 );
    Wait( 0.5 );
    robotDrive->drive( -0.5 , 0 );
    Wait( 0.5 );
    robotDrive->drive( 0 , 0 );
}

