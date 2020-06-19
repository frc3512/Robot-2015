// Copyright (c) 2015-2020 FRC Team 3512. All Rights Reserved.

#include <units/units.h>

#include "Robot.hpp"

using namespace std::chrono_literals;

enum class AutoState { Init, MoveForward, Idle };

static AutoState state;

void Robot::AutoMotionProfileInit() { state = AutoState::Init; }

void Robot::AutoMotionProfilePeriodic() {
    switch (state) {
        case AutoState::Idle:
            break;
        case AutoState::Init:
            drivetrain.SetLeftVoltage(0_V);
            drivetrain.SetRightVoltage(0_V);

            drivetrain.ResetEncoders();

            drivetrain.SetSetpointsToMeasurements();
            autoTimer.Reset();
            drivetrain.SetLeftGoal(150_in);
            drivetrain.SetRightGoal(150_in);
            state = AutoState::MoveForward;
            break;
        case AutoState::MoveForward:
            if (!drivetrain.LeftAtGoal() || !drivetrain.RightAtGoal()) {
                drivetrain.UpdateControllers();
            } else {
                // Stop moving
                drivetrain.SetLeftVoltage(0_V);
                drivetrain.SetRightVoltage(0_V);
                state = AutoState::Idle;
            }
            break;
    }
}
