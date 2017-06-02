// Copyright (c) 2015-2017 FRC Team 3512. All Rights Reserved.

#include "ButtonTracker.hpp"

#include <DriverStation.h>

ButtonTracker::ButtonTracker(uint32_t port) { m_port = port; }

void ButtonTracker::UpdateButtons() {
    // "new" values are now "old"
    m_oldStates = m_newStates;

    // get newer values
    m_newStates = DriverStation::GetInstance().GetStickButtons(m_port);
}

bool ButtonTracker::PressedButton(uint32_t button) {
    return GetButtonState(m_oldStates, button) ==
               false &&  // if button wasn't pressed
           GetButtonState(m_newStates, button) == true;  // and it is now
}

bool ButtonTracker::ReleasedButton(uint32_t button) {
    return GetButtonState(m_oldStates, button) ==
               true &&  // if button was pressed
           GetButtonState(m_newStates, button) == false;  // and it isn't now
}

bool ButtonTracker::GetButtonState(uint16_t& buttonStates, uint32_t& button) {
    return ((1 << (button - 1)) & buttonStates) != 0;
}
