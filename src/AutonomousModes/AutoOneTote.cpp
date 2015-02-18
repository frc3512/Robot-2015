// =============================================================================
// File Name: AutoOneTote.cpp
// Description: Drives forward and picks up one tote
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "../Robot.hpp"

void Robot::AutoOneTote() {

    ev->setManualMode(false);
    autonStart();

    while (IsAutonomous() && IsEnabled()) {
        DS_PrintOut();

        autonUpdateState();
        ev->updateState();
        //ev->intakeGrab(false);

        Wait(0.01);
    }
}

void Robot::autonStart() {
    if (m_autoState == STATE_IDLE) {
        m_autoState = STATE_SEEK_GARBAGECAN_UP;
        autonStateChanged(STATE_IDLE, m_autoState);
    }
}

void Robot::autonUpdateState() {
    if (m_autoState == STATE_SEEK_GARBAGECAN_UP && ev-> atGoal()) {
        m_autoState = STATE_MOVE_TO_TOTE;
        autonStateChanged(STATE_SEEK_GARBAGECAN_UP, m_autoState);
    }

    if (m_autoState == STATE_MOVE_TO_TOTE && autoTimer->HasPeriodPassed(1.0)) {
        robotDrive->drive(0, 0, false);
        m_autoState = STATE_AUTOSTACK;
        autonStateChanged(STATE_MOVE_TO_TOTE, m_autoState);
    }

    if (m_autoState == STATE_MOVE_TO_TOTE) {
        robotDrive->drive(-0.3, 0, false);
    }

    if (m_autoState == STATE_AUTOSTACK && autoTimer->HasPeriodPassed(1.0) /* !ev->isStacking() */) {
        m_autoState = STATE_TURN;
        autonStateChanged(STATE_AUTOSTACK, m_autoState);
    }

    if (m_autoState == STATE_TURN && autoTimer->HasPeriodPassed(1.4)) {
        robotDrive->drive(0, 0, false);
        m_autoState = STATE_RUN_AWAY;
        autonStateChanged(STATE_TURN, m_autoState);
    }

    if (m_autoState == STATE_TURN) {
        robotDrive->drive(-0.3, -0.3, true);
    }

    if (m_autoState == STATE_RUN_AWAY && autoTimer->HasPeriodPassed(5.0)) {
        robotDrive->drive(0, 0, false);
        m_autoState = STATE_IDLE;
        autonStateChanged(STATE_RUN_AWAY, m_autoState);
    }

    if (m_autoState == STATE_RUN_AWAY) {
        robotDrive->drive(-0.3, 0, false);
    }

    if(m_autoState == STATE_AUTOSTACK || m_autoState == STATE_TURN || m_autoState == STATE_RUN_AWAY) {
        ev->setIntakeDirection(Elevator::S_REVERSED);
    }

    // TODO: Do this in a state transition
    if(m_autoState == STATE_IDLE) {
        ev->setIntakeDirection(Elevator::S_STOPPED);
    }
}

void Robot::autonStateChanged(AutoState oldState, AutoState newState) {
    if (newState == STATE_SEEK_GARBAGECAN_UP) {
        ev->raiseElevator("EV_GARBAGECAN_LEVEL");
    }

    if (newState == STATE_MOVE_TO_TOTE) {
        autoTimer->Reset();
        autoTimer->Start();
    }

    if (newState == STATE_AUTOSTACK) {
        autoTimer->Reset();
        autoTimer->Start();
        ev->stowIntake(false);
        ev->intakeGrab(true);

        //ev->stackTotes();
    }

    if (newState == STATE_TURN) {
        autoTimer->Reset();
        autoTimer->Start();
    }

    if (newState == STATE_RUN_AWAY) {
        autoTimer->Reset();
        autoTimer->Start();
    }

}
