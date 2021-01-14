// Copyright (c) 2015-2021 FRC Team 3512. All Rights Reserved.

#include "subsystems/Elevator.hpp"

#include <utility>

#include <wpi/raw_ostream.h>

Elevator::Elevator() {
    State state{"IDLE"};
    state.entry = [this] { m_startAutoStacking = false; };
    state.transition = [this] {
        if (m_startAutoStacking) {
            return "WAIT_INITIAL_HEIGHT";
        } else {
            return "";
        }
    };
    m_autoStackSM.AddState(std::move(state));
    m_autoStackSM.SetState("IDLE");

    state = State{"WAIT_INITIAL_HEIGHT"};
    state.entry = [this] { SetGoal(kToteHeight1); };
    state.transition = [this] {
        if (AtGoal()) {
            return "SEEK_DROP_TOTES";
        } else {
            return "";
        }
    };
    m_autoStackSM.AddState(std::move(state));

    state = State{"SEEK_DROP_TOTES"};
    state.entry = [this] {
        SetGoal(m_controller.GetGoal().position - kAutoDropHeight);
    };
    state.transition = [this] {
        if (AtGoal()) {
            return "RELEASE";
        } else {
            return "";
        }
    };
    m_autoStackSM.AddState(std::move(state));

    state = State{"RELEASE"};
    state.entry = [this] {
        m_grabTimer.Reset();
        m_grabTimer.Start();
        ElevatorGrab(false);
    };
    state.transition = [this] {
        if (m_grabTimer.HasPeriodPassed(0.2_s)) {
            return "SEEK_GROUND";
        } else {
            return "";
        }
    };
    m_autoStackSM.AddState(std::move(state));

    state = State{"SEEK_GROUND"};
    state.entry = [this] { SetGoal(kGroundHeight); };
    state.transition = [this] {
        if (AtGoal()) {
            return "GRAB";
        } else {
            return "";
        }
    };
    m_autoStackSM.AddState(std::move(state));

    state = State{"GRAB"};
    state.entry = [this] {
        m_grabTimer.Reset();
        m_grabTimer.Start();
        ElevatorGrab(true);
    };
    state.transition = [this] {
        if (m_grabTimer.HasPeriodPassed(0.4_s)) {
            return "SEEK_HALF_TOTE";
        } else {
            return "";
        }
    };
    m_autoStackSM.AddState(std::move(state));

    state = State{"SEEK_HALF_TOTE"};
    state.entry = [this] { SetGoal(kToteHeight2); };
    state.transition = [this] {
        if (AtGoal()) {
            return "INTAKE_IN";
        } else {
            return "";
        }
    };
    m_autoStackSM.AddState(std::move(state));

    state = State{"INTAKE_IN"};
    state.entry = [this] {
        m_grabTimer.Reset();
        m_grabTimer.Start();
        IntakeGrab(true);
    };
    state.transition = [this] {
        if (m_grabTimer.HasPeriodPassed(0.2_s)) {
            return "IDLE";
        } else {
            return "";
        }
    };
    m_autoStackSM.AddState(std::move(state));
}

void Elevator::ElevatorGrab(bool state) { m_elevatorGrabber.Set(!state); }

bool Elevator::IsElevatorGrabbed() const { return !m_elevatorGrabber.Get(); }

void Elevator::IntakeGrab(bool state) { m_intakeGrabber.Set(state); }

bool Elevator::IsIntakeGrabbed() const { return m_intakeGrabber.Get(); }

void Elevator::StowIntake(bool state) { m_intakeStower.Set(!state); }

bool Elevator::IsIntakeStowed() const { return !m_intakeStower.Get(); }

void Elevator::ContainerGrab(bool state) { m_containerGrabber.Set(!state); }

bool Elevator::IsContainerGrabbed() const { return !m_containerGrabber.Get(); }

void Elevator::SetIntakeDirection(IntakeMotorState state) {
    m_intakeState = state;

    if (state == S_STOPPED) {
        m_intakeLeftMotor.Set(0);
        m_intakeRightMotor.Set(0);
    } else if (state == S_FORWARD) {
        m_intakeLeftMotor.Set(1);
        m_intakeRightMotor.Set(-1);
    } else if (state == S_REVERSE) {
        m_intakeLeftMotor.Set(-1);
        m_intakeRightMotor.Set(1);
    } else if (state == S_ROTATE_CCW) {
        m_intakeLeftMotor.Set(-1);
        m_intakeRightMotor.Set(-1);
    } else if (state == S_ROTATE_CW) {
        m_intakeLeftMotor.Set(1);
        m_intakeRightMotor.Set(1);
    }
}

Elevator::IntakeMotorState Elevator::GetIntakeDirection() const {
    return m_intakeState;
}

void Elevator::SetManualLiftSpeed(units::volt_t value) {
    if (m_manual) {
        m_liftGrbx.SetVoltage(value);
    }
}

units::meters_per_second_t Elevator::GetManualLiftSpeed() {
    if (m_manual) {
        return units::inch_t{m_liftEncoder.GetRate()} / 1_s;
    }
    return 0_mps;
}

void Elevator::SetManualMode(bool on) {
    if (on != m_manual) {
        m_manual = on;

        if (m_manual) {
            // Stop any auto-stacking when we switch to manual mode
            m_autoStackSM.SetState("IDLE");
        } else {
            SetGoal(GetHeight());
        }
    }
}

bool Elevator::IsManualMode() const { return m_manual; }

void Elevator::SetHeight(units::meter_t height) {
    if (m_manual == false) {
        m_controller.SetGoal(height);
    }
}

units::meter_t Elevator::GetHeight() {
    return units::inch_t{m_liftEncoder.GetDistance()};
}

void Elevator::ResetEncoders() { m_liftEncoder.Reset(); }

void Elevator::RaiseElevator(units::meter_t level) {
    /* Only allow changing the elevator height manually if not currently
     * auto-stacking
     */
    if (!IsStacking()) {
        wpi::outs() << "Seeking to " << level.to<double>() << "\n";
        SetGoal(level);
    }
}

void Elevator::StackTotes() {
    SetManualMode(false);
    m_startAutoStacking = true;
}

bool Elevator::IsStacking() const { return m_autoStackSM.GetState() != "IDLE"; }

void Elevator::CancelStack() { m_autoStackSM.SetState("IDLE"); }

void Elevator::UpdateState() {
    m_autoStackSM.run();

    /* Opens intake if the elevator is at the same level as it or if the tines
     * are open
     */
    if (IsIntakeGrabbed()) {
        if ((m_controller.GetSetpoint().position < 11_in && !IsManualMode()) ||
            !IsElevatorGrabbed() || IsIntakeStowed()) {
            IntakeGrab(false);
        }
    }

    // If elevator is at ground and wasn't before
    if (!m_lastLimitSwitchValue && m_limitSwitch.Get()) {
        m_liftEncoder.Reset();
        SetGoal(GetHeight());
    }
    m_liftGrbx.Set(
        m_controller.Calculate(units::inch_t{m_liftEncoder.GetDistance()}));

    m_lastLimitSwitchValue = m_limitSwitch.Get();
}

bool Elevator::AtGoal() const { return m_controller.AtGoal(); }

void Elevator::SetGoal(units::meter_t height) {
    if (height > kMaxHeight) {
        height = kMaxHeight;
    }

    // Set PID constant profile
    if (height > GetHeight()) {
        // Going up.
        m_controller.SetConstraints({kMaxVUp, kMaxAUp});
    } else {
        // Going down.
        if (height > 0_in) {
            m_controller.SetConstraints({kMaxVDown, kMaxADown});
        } else {
            m_controller.SetConstraints({kMaxVDownZeroing, kMaxADown});
            height = -100_in;
        }
    }

    m_controller.SetGoal(height);
}
