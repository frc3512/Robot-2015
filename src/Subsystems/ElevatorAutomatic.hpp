// =============================================================================
// File Name: ElevatorAutomatic.hpp
// Description: Implements elevator auto-stacking
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#ifndef ELEVATOR_AUTOMATIC_HPP
#define ELEVATOR_AUTOMATIC_HPP

#include "Elevator.hpp"
#include "../MotionProfile/TrapezoidProfile.hpp"
#include <vector>
#include <memory>
#include <thread>
#include <atomic>
#include <Timer.h>

class ElevatorAutomatic : public Elevator, public TrapezoidProfile {
public:
    enum ElevatorState {
        STATE_IDLE,
        STATE_WAIT_INITIAL_HEIGHT,
        STATE_SEEK_DROP_TOTES,
        STATE_RELEASE,
        STATE_SEEK_GROUND,
        STATE_GRAB,
        STATE_SEEK_HALF_TOTE
    };

    ElevatorAutomatic();
    virtual ~ElevatorAutomatic();

    void updateState();
    void raiseElevator(unsigned int numTotes);
    float getLevel(unsigned int i);
    void stackTotes();

private:
    void stateChanged(ElevatorState oldState, ElevatorState newState);

    std::vector<float> m_toteHeights;
    std::unique_ptr<Timer> m_profileTimer;
    std::atomic<bool> m_updateProfile;
    std::unique_ptr<std::thread> m_profileUpdater;

    std::unique_ptr<Elevator> m_elevator;
    ElevatorState m_state;
    std::unique_ptr<Timer> m_grabTimer;
    unsigned int m_ntotes;
};

#endif // ELEVATOR_AUTOMATIC_HPP

