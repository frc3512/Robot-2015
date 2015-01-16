#ifndef _ROBOT_HPP
#define _ROBOT_HPP

#include <SampleRobot.h>
#include <Joystick.h>
#include <Solenoid.h>
#include <Timer.h>

#include "Subsystems/DriveTrain.hpp"
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
    void Test();
    bool testDriveTrain( bool shifterState ,
                         bool direction ,
                         float lowerBound ,
                         float upperBound );
    void calibrateTalons();

    bool checkReflectiveStrips();

    void RightLeftAuton();
    void AutonMotionProfile();
    void DriveForwardAuton();
    void SideAuton();
    void NoopAuton();

    void DS_PrintOut();

private:
    Settings settings;

    DriveTrain* robotDrive;

    Joystick* driveStick1;
    Joystick* driveStick2;
    Joystick* shootStick;

    ButtonTracker drive1Buttons;
    ButtonTracker drive2Buttons;
    ButtonTracker shootButtons;

    Timer* autonTimer;
    Timer* displayTimer;

    // Used for sending data to the Driver Station
    DriverStationDisplay<Robot>* driverStation;

    // The LiveGrapher host
    GraphHost pidGraph;

    Insight* insight;

    Logger* logger1;
    LogFileSink* logFileSink;
    LogServerSink* logServerSink;
    LogStream* ls;
};

#endif // _ROBOT_HPP

