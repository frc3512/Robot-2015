// =============================================================================
// File Name: AutoOneTote.cpp
// Description: Drives forward and picks up one tote
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "../Robot.hpp"
#include "../StateMachine.hpp"

void Robot::AutoOneTote() {
    StateMachine oneToteSM;
    State* idle = new State("IDLE");
    idle->endFunc = [this] { ev->setIntakeDirection(Elevator::S_STOPPED); };

    oneToteSM.addState(idle);

    oneToteSM.addState(
        new State("SEEK_GARBAGECAN_UP",
                  [this] { ev->raiseElevator("EV_GARBAGECAN_LEVEL"); },
                  [this] () -> bool { return ev->atGoal(); }));

    oneToteSM.addState(
        new State("MOVE_TO_TOTE",
                  [this] {
        autoTimer->Reset();
        autoTimer->Start();
    },
                  [this] () -> bool { return autoTimer->HasPeriodPassed(1.0); },
                  [this] { robotDrive->drive(-0.3, 0, false); }));

    oneToteSM.addState(
        new State("AUTOSTACK",
                  [this] {
        autoTimer->Reset();
        autoTimer->Start();
        ev->stowIntake(false);
        ev->intakeGrab(true);
        ev->setIntakeDirection(Elevator::S_REVERSED);

        // ev->stackTotes();
    },
                  [this] () -> bool { return autoTimer->HasPeriodPassed(1.0); }
                  )
        );

    oneToteSM.addState(
        new State("TURN",
                  [this] {
        autoTimer->Reset();
        autoTimer->Start();
        ev->setIntakeDirection(Elevator::S_REVERSED);
    },
                  [this] { return autoTimer->HasPeriodPassed(1.4); },
                  [this] { robotDrive->drive(-0.3, -0.3, true); },
                  [this] { robotDrive->drive(0.0, 0.0, false); }));

    oneToteSM.addState(
        new State("RUN_AWAY",
                  [this] {
        autoTimer->Reset();
        autoTimer->Start();
        ev->setIntakeDirection(Elevator::S_REVERSED);
    },
                  [this] () -> bool { return autoTimer->HasPeriodPassed(5.0); },
                  [this] { robotDrive->drive(-0.3, 0, false); },
                  [this] { robotDrive->drive(0, 0, false); }));

    ev->setManualMode(false);
    oneToteSM.start();

    while (IsAutonomous() && IsEnabled()) {
        DS_PrintOut();

        oneToteSM.run();
        ev->updateState();
        // ev->intakeGrab(false);

        Wait(0.01);
    }
}

