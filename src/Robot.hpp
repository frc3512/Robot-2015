// Copyright (c) 2015-2017 FRC Team 3512. All Rights Reserved.

#pragma once

#include <Joystick.h>
#include <SampleRobot.h>
#include <Solenoid.h>
#include <Timer.h>

#include "ButtonTracker.hpp"
#include "DSDisplay.hpp"
#include "LiveGrapherHost/GraphHost.hpp"
#include "Settings.hpp"
#include "Subsystems/DriveTrain.hpp"
#include "Subsystems/Elevator.hpp"

/**
 * Implements the main robot class
 */
class Robot : public SampleRobot {
public:
    Robot();
    void OperatorControl();
    void Autonomous();
    void Disabled();

    void AutoNoop();
    void AutoDriveForward();
    void AutoResetElevator();
    void AutoOneCanLeft();
    void AutoOneCanCenter();
    void AutoOneCanRight();
    void AutoOneTote();
    void AutoMotionProfile();

    void DS_PrintOut();

    static float ApplyDeadband(float value, float deadband);

private:
    Settings settings{"/home/lvuser/RobotSettings.txt"};

    DriveTrain robotDrive;
    Elevator ev;

    Joystick driveStick1{0};
    Joystick driveStick2{1};
    Joystick evStick{2};

    ButtonTracker drive1Buttons{0};
    ButtonTracker drive2Buttons{1};
    ButtonTracker evButtons{2};

    Timer autoTimer;
    Timer displayTimer;

    // Used for sending data to the Driver Station
    DSDisplay& dsDisplay{DSDisplay::GetInstance(settings.GetInt("DS_Port"))};

    // The LiveGrapher host
    GraphHost pidGraph{3513};
};
