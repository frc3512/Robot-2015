// Copyright (c) 2020-2021 FRC Team 3512. All Rights Reserved.

#pragma once

#include <ctre/phoenix/motorcontrol/can/TalonSRX.h>
#include <frc/SpeedController.h>

class TalonSRXGroup : public frc::SpeedController {
public:
    template <class... Talons>
    explicit TalonSRXGroup(ctre::phoenix::motorcontrol::can::TalonSRX& leader,
                           Talons&... followers)
        : m_leader{leader} {
        FollowImpl(followers...);
    }

    TalonSRXGroup(TalonSRXGroup&&) = default;
    TalonSRXGroup& operator=(TalonSRXGroup&&) = default;

    void Set(double speed) override;
    double Get() const override;
    void SetInverted(bool isInverted) override;
    bool GetInverted() const override;
    void Disable() override;
    void StopMotor() override;
    void PIDWrite(double output) override;

private:
    double m_speed = 0.0;
    bool m_isInverted = false;
    ctre::phoenix::motorcontrol::can::TalonSRX& m_leader;

    template <class Talon, class... Talons>
    void FollowImpl(Talon& follower, Talons&... followers) {
        follower.Follow(m_leader);

        if constexpr (sizeof...(followers) > 0) {
            FollowImpl(followers...);
        }
    }
};
