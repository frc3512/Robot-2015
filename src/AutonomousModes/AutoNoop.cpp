#include "../Robot.hpp"
#include <Timer.h>

void Robot::AutoNoop() {
    while (IsAutonomous() && IsEnabled()) {
        DS_PrintOut();

        Wait(0.01);
    }
}

