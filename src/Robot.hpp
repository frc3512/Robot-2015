#ifndef ROBOT_HPP
#define ROBOT_HPP

#include <memory>

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
    virtual ~Robot();
    void OperatorControl();
    void Autonomous();
    void Disabled();

    void AutoDriveForward();
    void AutoOneTote();
    void AutoNoop();

    void DS_PrintOut();

    enum AutoState {
      STATE_IDLE,
      STATE_SEEK_GARBAGECAN_UP,
      STATE_MOVE_TO_TOTE,
      STATE_AUTOSTACK,
      STATE_TURN,
      STATE_RUN_AWAY
    };

    AutoState m_autoState;

    void autonStart();
    void autonUpdateState();
    void autonStateChanged(AutoState oldState, AutoState newState);
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

    RollingAverage<double> manualAverage;

    // Used for sending data to the Driver Station
    DSDisplay& dsDisplay;

    // The LiveGrapher host
    GraphHost pidGraph;
};

#endif // ROBOT_HPP

