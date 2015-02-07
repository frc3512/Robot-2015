/*
 * ElevatorAutomatic.hpp
 *
 *  Created on: Feb 6, 2015
 *      Author: amelia
 */

#ifndef SRC_SUBSYSTEMS_ELEVATORAUTOMATIC_HPP_
#define SRC_SUBSYSTEMS_ELEVATORAUTOMATIC_HPP_

#include "Elevator.hpp"
#include "Timer.h"

class ElevatorAutomatic {
public:
	enum ElevatorState {
		STATE_IDLE,
		STATE_BUTTON_DONE,
		STATE_INITIAL_SEEK_DONE,
		STATE_INITIAL_SEEK,
		STATE_SECOND_SEEK,
		STATE_GRAB,
		STATE_THIRD_SEEK,
		STATE_RELEASE,
		STATE_FOURTH_SEEK
	};

	ElevatorAutomatic(Elevator *elevator);
	virtual ~ElevatorAutomatic();

	void updateState();
	void raiseElevator(int numTotes);
	void stackTotes();

private:
	void stateChanged(ElevatorState oldState, ElevatorState newState);

	const float m_toteHeights[5]{100, 150, 180, 200, 210};

	Elevator *m_elevator;
	ElevatorState m_state;
	Timer *m_timer;
	int m_ntotes;

};

#endif /* SRC_SUBSYSTEMS_ELEVATORAUTOMATIC_HPP_ */
