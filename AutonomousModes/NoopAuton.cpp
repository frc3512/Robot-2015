#include "../Robot.hpp"
#include <Timer.h>

void Robot::NoopAuton(){
    while ( IsAutonomous() ) {
        Wait( 0.01 );
    }
}

