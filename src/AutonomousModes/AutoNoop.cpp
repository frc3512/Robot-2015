// Copyright (c) FRC Team 3512, Spartatroniks 2015-2016. All Rights Reserved.

#include "../Robot.hpp"

// Does nothing
void Robot::AutoNoop() {
    while (IsAutonomous() && IsEnabled()) {
        DS_PrintOut();

        std::this_thread::sleep_for(10ms);
    }
}
