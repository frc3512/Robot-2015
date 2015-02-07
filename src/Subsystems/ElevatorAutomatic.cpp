/*
 * ElevatorAutomatic.cpp
 *
 *  Created on: Feb 6, 2015
 *      Author: amelia
 */

#include "Subsystems/ElevatorAutomatic.hpp"

ElevatorAutomatic::ElevatorAutomatic(Elevator* elevator) {
    m_elevator = elevator;
    m_timer = new Timer();
    m_state = STATE_IDLE;
    m_ntotes = 0;
}

ElevatorAutomatic::~ElevatorAutomatic() {
}

void ElevatorAutomatic::updateState() {
    if (m_elevator->onTarget() && m_state == STATE_INITIAL_SEEK) {
        m_state = STATE_INITIAL_SEEK_DONE;
        stateChanged(STATE_INITIAL_SEEK, STATE_INITIAL_SEEK_DONE);
    }
    else if (m_elevator->onTarget() && m_state == STATE_BUTTON_DONE) {
        m_state = STATE_SECOND_SEEK;
        stateChanged(STATE_INITIAL_SEEK, STATE_SECOND_SEEK);
    }
    else if (m_elevator->onTarget() && m_state == STATE_SECOND_SEEK) {
        m_state = STATE_RELEASE;
        stateChanged(STATE_SECOND_SEEK, STATE_RELEASE);
    }
    else if (m_timer->HasPeriodPassed(5) && m_state == STATE_RELEASE) {
        m_state = STATE_THIRD_SEEK;
        stateChanged(STATE_RELEASE, STATE_THIRD_SEEK);
    }
    else if (m_elevator->onTarget() && m_state == STATE_THIRD_SEEK) {
        m_state = STATE_GRAB;
        stateChanged(STATE_THIRD_SEEK, STATE_GRAB);
    }
    else if (m_timer->HasPeriodPassed(5) && m_state == STATE_GRAB) {
        m_state = STATE_FOURTH_SEEK;
        stateChanged(STATE_GRAB, STATE_FOURTH_SEEK);
    }
    else if (m_elevator->onTarget() && m_state == STATE_FOURTH_SEEK) {
        m_state = STATE_IDLE;
        stateChanged(STATE_FOURTH_SEEK, STATE_IDLE);
    }
}

void ElevatorAutomatic::raiseElevator(int numTotes) {
    // Bail out if numTotes is invalid
    if (numTotes < 0 || numTotes > 4) {
        return;
    }

    /* Make sure we're in the correct state */
    if (m_state != STATE_IDLE) {
        return;
    }

    m_ntotes = numTotes;

    m_state = STATE_INITIAL_SEEK;
    stateChanged(STATE_IDLE, STATE_INITIAL_SEEK);
}

void ElevatorAutomatic::stackTotes() {
    if (m_state == STATE_INITIAL_SEEK) {
        m_state = STATE_BUTTON_DONE;
        stateChanged(STATE_INITIAL_SEEK, STATE_BUTTON_DONE);
    }
    else if (m_state == STATE_INITIAL_SEEK_DONE) {
        m_state = STATE_SECOND_SEEK;
        stateChanged(STATE_INITIAL_SEEK_DONE, STATE_SECOND_SEEK);
    }
}

void ElevatorAutomatic::stateChanged(ElevatorState oldState,
                                     ElevatorState newState) {
    if (oldState == STATE_IDLE && newState == STATE_INITIAL_SEEK) {
        // Start the elevator seeking to the correct height
        m_elevator->setHeight(m_toteHeights[(m_ntotes * 2) + 1]);

        // Update the state machine
    }

    /* Don't care about oldState. The transition could have occurred from
     * either STATE_BUTTON_DONE or STATE_INITIAL_SEEK_DONE */
    if (newState == STATE_SECOND_SEEK) {
        m_elevator->setHeight(m_toteHeights[(m_ntotes * 2)]);
    }

    /* Grab the tote */
    if (newState == STATE_RELEASE) {
        m_timer->Reset();
        m_timer->Start();
        m_elevator->elevatorGrab(false);
    }

    /* All the way to the bottom */
    if (newState == STATE_THIRD_SEEK) {
        m_elevator->setHeight(m_toteHeights[0]);
    }

    /* Grab the new stack */
    if (newState == STATE_GRAB) {
        m_timer->Reset();
        m_timer->Start();
        m_elevator->elevatorGrab(true);
    }

    /* Off the ground a bit */
    if (newState == STATE_FOURTH_SEEK) {
        m_elevator->setHeight(m_toteHeights[1]);
    }
}

