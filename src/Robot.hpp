// =============================================================================
// File Name: Robot.hpp
// Description: Implements the main robot class
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#ifndef ROBOT_HPP
#define ROBOT_HPP

#include <chrono>
using namespace std::chrono_literals;

#include <SampleRobot.h>
#include <Joystick.h>
#include <Solenoid.h>
#include <Timer.h>

#include "Subsystems/DriveTrain.hpp"
#include "Subsystems/Elevator.hpp"
#include "ButtonTracker.hpp"
#include "Settings.hpp"

#include "DSDisplay.hpp"
#include "LiveGrapherHost/GraphHost.hpp"

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

#endif // ROBOT_HPP

