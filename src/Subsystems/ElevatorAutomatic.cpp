// =============================================================================
// File Name: ElevatorAutomatic.cpp
// Description: Provides an interface for the robot's elevator
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "Subsystems/ElevatorAutomatic.hpp"

ElevatorAutomatic::ElevatorAutomatic() {
    m_grabTimer = std::make_unique<Timer>();
    m_state = STATE_IDLE;
    m_ntotes = 0;

    m_toteHeights.push_back(0.0);

    float height = 0;
    //TODO: magic number
    for (int i = 0; i < 13; i++) {
    	height = m_settings->getFloat("EV_LEVEL_" + std::to_string(i));
        m_toteHeights.push_back(height);
    }
}

ElevatorAutomatic::~ElevatorAutomatic() {
}

void ElevatorAutomatic::updateState() {
    if (onTarget() && m_state == STATE_WAIT_INITIAL_HEIGHT) {
        stateChanged(STATE_WAIT_INITIAL_HEIGHT,
                     m_state = STATE_SEEK_DROP_TOTES);
    }
    if (onTarget() && m_state == STATE_SEEK_DROP_TOTES) {
        m_state = STATE_RELEASE;
        stateChanged(STATE_SEEK_DROP_TOTES, m_state);
    }
    else if (m_grabTimer->HasPeriodPassed(5) && m_state == STATE_RELEASE) {
        m_state = STATE_SEEK_GROUND;
        stateChanged(STATE_RELEASE, m_state);
    }
    else if (onTarget() && m_state == STATE_SEEK_GROUND) {
        m_state = STATE_GRAB;
        stateChanged(STATE_SEEK_GROUND, m_state);
    }
    else if (m_grabTimer->HasPeriodPassed(5) && m_state == STATE_GRAB) {
        m_state = STATE_SEEK_HALF_TOTE;
        stateChanged(STATE_GRAB, m_state);
    }
    else if (onTarget() && m_state == STATE_SEEK_HALF_TOTE) {
        m_state = STATE_IDLE;
        stateChanged(STATE_SEEK_HALF_TOTE, m_state);
    }
}

void ElevatorAutomatic::raiseElevator(unsigned int numTotes) {
    // Bail out if numTotes is invalid
    if (numTotes < 0 || numTotes > m_toteHeights.size()) {
        return;
    }

    std::cout << "m_toteHeights[" << numTotes * 2 << "] == "
    		<< m_toteHeights[numTotes * 2] << std::endl;

    /* Only allow changing the elevator height manually if not currently
     * auto-stacking
     */
    if (m_state == STATE_IDLE) {
    	std::cout << "Seeking to " << m_toteHeights[numTotes * 2] << std::endl;
        setHeight(m_toteHeights[numTotes * 2]);
        m_ntotes = numTotes;
    }
}

float ElevatorAutomatic::getLevel(unsigned int i) {
	if(i * 2 < m_toteHeights.size()) {
		return m_toteHeights[i * 2];
	}

	return 0.0;
}

void ElevatorAutomatic::stackTotes() {
    if (m_state == STATE_IDLE) {
        m_state = STATE_WAIT_INITIAL_HEIGHT;
        stateChanged(STATE_IDLE, m_state);
    }
}

void ElevatorAutomatic::stateChanged(ElevatorState oldState,
                                     ElevatorState newState) {

	std::cout << "oldState = " << stateToString(oldState)
			<< " newState = " << stateToString(newState) << std::endl;
    if (newState == STATE_SEEK_DROP_TOTES) {
        setHeight(m_toteHeights[m_ntotes * 2]);
    }

    // Release the totes
    if (newState == STATE_RELEASE) {
        m_grabTimer->Reset();
        m_grabTimer->Start();
        elevatorGrab(true);
    }

    if (newState == STATE_SEEK_GROUND) {
        setHeight(m_toteHeights[0]);
    }

    // Grab the new stack
    if (newState == STATE_GRAB) {
        m_grabTimer->Reset();
        m_grabTimer->Start();
        elevatorGrab(false);
    }

    // Off the ground a bit
    if (newState == STATE_SEEK_HALF_TOTE) {
        setHeight(m_toteHeights[1]);
    }
}

std::string ElevatorAutomatic::stateToString(ElevatorState state) {
	switch(state) {
	case STATE_IDLE:
		return "STATE_IDLE";
	case STATE_WAIT_INITIAL_HEIGHT:
		return "STATE_WAIT_INITIAL_HEIGHT";
	case STATE_SEEK_DROP_TOTES:
		return "STATE_SEEK_DROP_TOTES";
	case STATE_RELEASE:
		return "STATE_RELEASE";
	case STATE_SEEK_GROUND:
		return "STATE_SEEK_GROUND";
	case STATE_GRAB:
		return "STATE_GRAB";
	case STATE_SEEK_HALF_TOTE:
		return "STATE_SEEK_HALF_TOTE";
	}

	return "UNKNOWN STATE";
}
