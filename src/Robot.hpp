#ifndef _ROBOT_HPP
#define _ROBOT_HPP

#include <memory>

#include <SampleRobot.h>
#include <Joystick.h>
#include <Solenoid.h>
#include <Timer.h>

#include "Subsystems/DriveTrain.hpp"
#include "Subsystems/ElevatorAutomatic.hpp"
#include "ButtonTracker.hpp"
#include "Settings.hpp"

#include "DriverStationDisplay.hpp"
#include "LiveGrapherHost/GraphHost.hpp"
#include "Insight.hpp"

#include "Logging/LogConsoleSink.hpp"
#include "Logging/LogFileSink.hpp"
#include "Logging/Logger.hpp"
#include "Logging/LogServerSink.hpp"
#include "Logging/LogStream.hpp"
#include "Logging/SetLogLevel.hpp"

class Robot : public SampleRobot {
public:
    Robot();
    virtual ~Robot();
    void OperatorControl();
    void Autonomous();
    void Disabled();

    void AutonMotionProfile();
    void NoopAuton();

    void DS_PrintOut();

private:
    Settings settings;

    std::unique_ptr<DriveTrain> robotDrive;

    std::unique_ptr<Joystick> driveStick1;
    std::unique_ptr<Joystick> driveStick2;
    std::unique_ptr<Joystick> shootStick;

    ButtonTracker drive1Buttons;
    ButtonTracker drive2Buttons;
    ButtonTracker elevatorButtons;

    std::unique_ptr<Timer> autonTimer;
    std::unique_ptr<Timer> displayTimer;

    // Used for sending data to the Driver Station
    DriverStationDisplay& dsDisplay;
    Insight& insight;

    // The LiveGrapher host
    GraphHost pidGraph;

    std::unique_ptr<ElevatorAutomatic> ev;
    Logger* logger1;
    LogFileSink* logFileSink;
    LogServerSink* logServerSink;
    std::unique_ptr<LogStream> ls;
};

#endif // _ROBOT_HPP

