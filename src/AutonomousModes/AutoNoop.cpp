#include "../Robot.hpp"

void Robot::AutoNoop() {
    while (IsAutonomous() && IsEnabled()) {
        DS_PrintOut();

        Wait(0.01);
    }
}

