// Copyright (c) FRC Team 3512, Spartatroniks 2015-2016. All Rights Reserved.

#include "Elevator.hpp"

#include <memory>
#include <utility>

#include <CANTalon.h>
#include <Solenoid.h>

Elevator::Elevator() : TrapezoidProfile(0.0, 0.0) {
    m_intakeState = S_STOPPED;

    // For CANTalon PID loop
    m_liftGrbx.setEncoderReversed(true);
    m_liftGrbx.setDistancePerPulse(70.5 / 5090.0);

    m_liftGrbx.setProfile(false);
    m_liftGrbx.setIZone(80);
    m_liftGrbx.setCloseLoopRampRate(1.0);

    m_liftGrbx.setProfile(true);
    m_liftGrbx.setIZone(80);
    m_liftGrbx.setCloseLoopRampRate(1.0);

    m_profileUpdater = std::thread([this] {
        double height = 0.0;
        while (m_updateProfile) {
            if (!atGoal()) {
                height = updateSetpoint(m_profileTimer.Get());
            } else {
                height = m_setpoint;
            }
            setHeight(height);

            // If elevator is at ground
            if (m_liftGrbx.isRevLimitSwitchClosed()) {
                // If elevator wasn't before
                if (!m_wasAtGround) {
                    m_liftGrbx.resetEncoder();
                    height = 0.0;
                    setGoal(m_profileTimer.Get(), height, getHeight());

                    m_wasAtGround = true;
                }
            } else if (m_wasAtGround) {
                /* Elevator isn't at ground anymore. If it was previously on
                 * ground
                 */
                m_wasAtGround = false;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });

    // Load motion profile constants from the configuration file
    m_maxv_a = m_settings.GetDouble("EV_MAX_VELOCITY_PROFILE_A");
    m_ttmaxv_a = m_settings.GetDouble("EV_TIME_TO_MAX_VELOCITY_PROFILE_A");
    m_maxv_b = m_settings.GetDouble("EV_MAX_VELOCITY_PROFILE_B");
    m_ttmaxv_b = m_settings.GetDouble("EV_TIME_TO_MAX_VELOCITY_PROFILE_B");

    // Load elevator levels from the configuration file
    m_maxHeight = m_settings.GetDouble("EV_MAX_HEIGHT");
    m_toteHeights["EV_GROUND"] = m_settings.GetDouble("EV_GROUND");

    double height = 0;
    for (int i = 1; i <= 6; i++) {
        height = m_settings.GetDouble("EV_TOTE_" + std::to_string(i));
        m_toteHeights["EV_TOTE_" + std::to_string(i)] = height;
    }

    m_toteHeights["EV_STEP"] = m_settings.GetDouble("EV_STEP");
    m_toteHeights["EV_HALF_TOTE_OFFSET"] =
        m_settings.GetDouble("EV_HALF_TOTE_OFFSET");
    m_toteHeights["EV_GARBAGECAN_LEVEL"] =
        m_settings.GetDouble("EV_GARBAGECAN_LEVEL");
    m_toteHeights["EV_AUTO_DROP_LENGTH"] =
        m_settings.GetDouble("EV_AUTO_DROP_LENGTH");

    auto state = std::make_unique<State>("IDLE");
    state->entry = [this] { m_startAutoStacking = false; };
    state->transition = [this] {
        if (m_startAutoStacking) {
            return "WAIT_INITIAL_HEIGHT";
        } else {
            return "";
        }
    };
    m_autoStackSM.AddState(std::move(state));
    m_autoStackSM.SetState("IDLE");

    state = std::make_unique<State>("WAIT_INITIAL_HEIGHT");
    state->entry = [this] { setProfileHeight(m_toteHeights["EV_TOTE_1"]); };
    state->transition = [this] {
        if (atGoal()) {
            return "SEEK_DROP_TOTES";
        } else {
            return "";
        }
    };
    m_autoStackSM.AddState(std::move(state));

    state = std::make_unique<State>("SEEK_DROP_TOTES");
    state->entry = [this] {
        setProfileHeight(getGoal() - m_toteHeights["EV_AUTO_DROP_LENGTH"]);
    };
    state->transition = [this] {
        if (atGoal()) {
            return "RELEASE";
        } else {
            return "";
        }
    };
    m_autoStackSM.AddState(std::move(state));

    state = std::make_unique<State>("RELEASE");
    state->entry = [this] {
        m_grabTimer.Reset();
        m_grabTimer.Start();
        elevatorGrab(false);
    };
    state->transition = [this] {
        if (m_grabTimer.HasPeriodPassed(0.2)) {
            return "SEEK_GROUND";
        } else {
            return "";
        }
    };
    m_autoStackSM.AddState(std::move(state));

    state = std::make_unique<State>("SEEK_GROUND");
    state->entry = [this] { setProfileHeight(m_toteHeights["EV_GROUND"]); };
    state->transition = [this] {
        if (atGoal()) {
            return "GRAB";
        } else {
            return "";
        }
    };
    m_autoStackSM.AddState(std::move(state));

    state = std::make_unique<State>("GRAB");
    state->entry = [this] {
        m_grabTimer.Reset();
        m_grabTimer.Start();
        elevatorGrab(true);
    };
    state->transition = [this] {
        if (m_grabTimer.HasPeriodPassed(0.4)) {
            return "SEEK_HALF_TOTE";
        } else {
            return "";
        }
    };
    m_autoStackSM.AddState(std::move(state));

    state = std::make_unique<State>("SEEK_HALF_TOTE");
    state->entry = [this] { setProfileHeight(m_toteHeights["EV_TOTE_2"]); };
    state->transition = [this] {
        if (atGoal()) {
            return "INTAKE_IN";
        } else {
            return "";
        }
    };
    m_autoStackSM.AddState(std::move(state));

    state = std::make_unique<State>("INTAKE_IN");
    state->entry = [this] {
        m_grabTimer.Reset();
        m_grabTimer.Start();
        intakeGrab(true);
    };
    state->transition = [this] {
        if (m_grabTimer.HasPeriodPassed(0.2)) {
            return "IDLE";
        } else {
            return "";
        }
    };
    m_autoStackSM.AddState(std::move(state));

    // Reload PID constants from the configuration file
    reloadPID();
}

Elevator::~Elevator() {
    m_updateProfile = false;
    m_profileUpdater.join();
}

void Elevator::elevatorGrab(bool state) { m_elevatorGrabber.Set(!state); }

bool Elevator::isElevatorGrabbed() const { return !m_elevatorGrabber.Get(); }

void Elevator::intakeGrab(bool state) { m_intakeGrabber.Set(state); }

bool Elevator::isIntakeGrabbed() const { return m_intakeGrabber.Get(); }

void Elevator::stowIntake(bool state) { m_intakeStower.Set(!state); }

bool Elevator::isIntakeStowed() const { return !m_intakeStower.Get(); }

void Elevator::containerGrab(bool state) { m_containerGrabber.Set(!state); }

bool Elevator::isContainerGrabbed() const { return !m_containerGrabber.Get(); }

void Elevator::setIntakeDirectionLeft(IntakeMotorState state) {
    m_intakeState = state;

    if (state == S_STOPPED) {
        m_intakeWheelLeft.Set(0);
    } else if (state == S_FORWARD) {
        m_intakeWheelLeft.Set(1);
    } else if (state == S_REVERSE) {
        m_intakeWheelLeft.Set(-1);
    } else if (state == S_ROTATE_CCW) {
        m_intakeWheelLeft.Set(-1);
    } else if (state == S_ROTATE_CW) {
        m_intakeWheelLeft.Set(1);
    }
}

void Elevator::setIntakeDirectionRight(IntakeMotorState state) {
    m_intakeState = state;

    if (state == S_STOPPED) {
        m_intakeWheelRight.Set(0);
    } else if (state == S_FORWARD) {
        m_intakeWheelRight.Set(-1);
    } else if (state == S_REVERSE) {
        m_intakeWheelRight.Set(1);
    } else if (state == S_ROTATE_CCW) {
        m_intakeWheelRight.Set(-1);
    } else if (state == S_ROTATE_CW) {
        m_intakeWheelRight.Set(1);
    }
}

Elevator::IntakeMotorState Elevator::getIntakeDirection() const {
    return m_intakeState;
}

void Elevator::setManualLiftSpeed(double value) {
    if (m_manual) {
        m_liftGrbx.setManual(value);
    }
}

double Elevator::getManualLiftSpeed() const {
    if (m_manual) {
        return m_liftGrbx.get(Grbx::Raw);
    }

    return 0.0;
}

void Elevator::setManualMode(bool on) {
    if (on != m_manual) {
        m_manual = on;

        if (m_manual) {
            // Stop any auto-stacking when we switch to manual mode
            m_autoStackSM.SetState("IDLE");
        } else {
            setProfileHeight(getHeight());
        }
    }
}

bool Elevator::isManualMode() const { return m_manual; }

void Elevator::setHeight(double height) {
    if (m_manual == false) {
        m_liftGrbx.setSetpoint(height);
    }
}

double Elevator::getHeight() const { return m_liftGrbx.get(Grbx::Position); }

void Elevator::reloadPID() {
    m_settings.Update();

    // First profile
    double p = 0.f;
    double i = 0.f;
    double d = 0.f;
    double f = 0.f;

    // Set elevator PID
    p = m_settings.GetDouble("PID_ELEVATOR_DOWN_P");
    i = m_settings.GetDouble("PID_ELEVATOR_DOWN_I");
    d = m_settings.GetDouble("PID_ELEVATOR_DOWN_D");
    f = m_settings.GetDouble("PID_ELEVATOR_DOWN_F");

    m_liftGrbx.setProfile(false);
    m_liftGrbx.setPID(p, i, d);
    m_liftGrbx.setF(f);

    // Set elevator PID
    p = m_settings.GetDouble("PID_ELEVATOR_UP_P");
    i = m_settings.GetDouble("PID_ELEVATOR_UP_I");
    d = m_settings.GetDouble("PID_ELEVATOR_UP_D");
    f = m_settings.GetDouble("PID_ELEVATOR_UP_F");

    m_liftGrbx.setProfile(true);
    m_liftGrbx.setPID(p, i, d);
    m_liftGrbx.setF(f);
}

void Elevator::resetEncoders() { m_liftGrbx.resetEncoder(); }

void Elevator::raiseElevator(std::string level) {
    size_t op = 0;
    size_t pos = 0;
    size_t nextPos = 0;
    double height = 0;
    decltype(m_toteHeights)::iterator it;

    bool firstNumber = true;
    while (pos != std::string::npos) {
        if (!firstNumber) {
            op = level.find_first_of("+-", pos);
            if (op == std::string::npos) {
                break;
            }
        } else {
            /* There is no operator associated with the first number, so keep
             * 'op' 0. This special case will be checked below so the number
             * is added to the total
             */
            firstNumber = false;
        }

        pos = level.find_first_not_of("+- ", op);
        if (pos == std::string::npos) {
            break;
        }
        nextPos = level.find_first_of("+- ", pos);

        it = m_toteHeights.find(level.substr(pos, nextPos - pos));
        if (it != m_toteHeights.end()) {
            if (level[op] == '+' || op == 0) {
                height += it->second;
            } else if (level[op] == '-') {
                height -= it->second;
            }
        }

        pos = nextPos;
    }

    /* Only allow changing the elevator height manually if not currently
     * auto-stacking
     */
    if (!isStacking()) {
        std::cout << "Seeking to " << height << std::endl;

        setProfileHeight(height);
    }
}

void Elevator::setProfileHeight(double height) {
    // Don't try to seek anywhere if we're already at setpoint
    /* if (height == getHeight()) {
     *   return;
     *  } */

    if (height > m_maxHeight) {
        height = m_maxHeight;
    }

    // Set PID constant profile
    if (height > getHeight()) {
        // Going up.
        setMaxVelocity(88.0);
        setTimeToMaxV(0.4);
        m_liftGrbx.setProfile(true);
    } else {
        // Going down.
        if (height > 0.0) {
            setMaxVelocity(91.26);
        } else {
            setMaxVelocity(45.63);
            height = -100.0;
        }
        setTimeToMaxV(0.4);
        m_liftGrbx.setProfile(false);
    }

    m_profileTimer.Reset();
    m_profileTimer.Start();

    setGoal(m_profileTimer.Get(), height, getHeight());
}

double Elevator::getLevelHeight(std::string level) const {
    auto height = m_toteHeights.find(level);

    if (height == m_toteHeights.end()) {
        return 0.0;
    } else {
        return height->second;
    }
}

void Elevator::stackTotes() {
    setManualMode(false);
    m_startAutoStacking = true;
}

bool Elevator::isStacking() const { return m_autoStackSM.GetState() != "IDLE"; }

void Elevator::cancelStack() { m_autoStackSM.SetState("IDLE"); }

void Elevator::updateState() {
    m_autoStackSM.run();

    /* Opens intake if the elevator is at the same level as it or if the tines
     * are open
     */
    if (isIntakeGrabbed()) {
        if ((getSetpoint() < 11 && !isManualMode()) || !isElevatorGrabbed() ||
            isIntakeStowed()) {
            intakeGrab(false);
        }
    }
}

void Elevator::manualChangeSetpoint(double delta) {
    double newSetpoint = delta + m_setpoint;

    if (newSetpoint > m_maxHeight) {
        newSetpoint = m_maxHeight;
    }

    // Set PID constant profile
    if (newSetpoint > m_setpoint) {
        // Going up.
        setMaxVelocity(m_maxv_a);
        setTimeToMaxV(m_ttmaxv_a);
        m_liftGrbx.setProfile(true);
    } else {
        // Going down.
        setMaxVelocity(m_maxv_b);
        setTimeToMaxV(m_ttmaxv_b);
        m_liftGrbx.setProfile(false);
    }

    m_setpoint = newSetpoint;
}
