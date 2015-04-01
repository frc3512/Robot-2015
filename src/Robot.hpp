// =============================================================================
// File Name: Robot.hpp
// Description: Implements the main robot class
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#ifndef ROBOT_HPP
#define ROBOT_HPP

#include <memory>
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

#include "RollingAverage.hpp"
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
    void AutoOneTote();
    void AutoMotionProfile();

    void DS_PrintOut();

    static float applyDeadband(float value, float deadband);

private:
    Settings settings;

    std::unique_ptr<DriveTrain> robotDrive;
    std::unique_ptr<Elevator> ev;

    std::unique_ptr<Joystick> driveStick1;
    std::unique_ptr<Joystick> driveStick2;
    std::unique_ptr<Joystick> evStick;

    ButtonTracker drive1Buttons;
    ButtonTracker drive2Buttons;
    ButtonTracker evButtons;

    std::unique_ptr<Timer> autoTimer;
    std::unique_ptr<Timer> displayTimer;
    std::unique_ptr<Timer> accumTimer;

    RollingAverage<double, 5> manualAverage;

    // Used for sending data to the Driver Station
    DSDisplay& dsDisplay;

    // The LiveGrapher host
    GraphHost pidGraph;
};

#endif // ROBOT_HPP

