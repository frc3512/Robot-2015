// Copyright (c) FRC Team 3512, Spartatroniks 2015-2016. All Rights Reserved.

#pragma once

template <typename Rep, typename Period>
void GraphHost::SetSendInterval(const duration<Rep, Period>& time) {
    m_sendInterval = duration_cast<milliseconds>(time).count();
}
