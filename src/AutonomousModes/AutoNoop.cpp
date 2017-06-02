// Copyright (c) 2015-2017 FRC Team 3512. All Rights Reserved.

#include "../Robot.hpp"

using namespace std::chrono_literals;

// Does nothing
void Robot::AutoNoop() {
    while (IsAutonomous() && IsEnabled()) {
        DS_PrintOut();

        std::this_thread::sleep_for(10ms);
    }
}
