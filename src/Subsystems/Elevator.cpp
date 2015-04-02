// =============================================================================
// File Name: Elevator.cpp
// Description: Provides an interface for the robot's elevator
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "Elevator.hpp"
#include <Solenoid.h>
#include <DigitalInput.h>
#include <CANTalon.h>

Elevator::Elevator() : TrapezoidProfile(0.0, 0.0) {
    m_elevatorGrabber = std::make_unique<Solenoid>(3);
    m_containerGrabber = std::make_unique<Solenoid>(4);

    m_intakeStower = std::make_unique<Solenoid>(1);
    m_intakeGrabber = std::make_unique<Solenoid>(2);
    m_intakeWheelLeft = std::make_unique<CANTalon>(3);
    m_intakeWheelRight = std::make_unique<CANTalon>(6);
    m_intakeState = S_STOPPED;
    m_manual = false;

    // For CANTalon PID loop
    m_liftGrbx = std::make_unique<GearBox<CANTalon>>(-1, 7, 2);
    m_liftGrbx->setEncoderReversed(true);
    m_liftGrbx->setDistancePerPulse(70.5 / 5090.0);

    m_liftGrbx->setProfile(false);
    m_liftGrbx->setIZone(80);
    m_liftGrbx->setCloseLoopRampRate(1.0);

    m_liftGrbx->setProfile(true);
    m_liftGrbx->setIZone(80);
    m_liftGrbx->setCloseLoopRampRate(1.0);

    m_profileTimer = std::make_unique<Timer>();
    m_grabTimer = std::make_unique<Timer>();
    m_updateProfile = true;

    m_setpoint = 0.0;

    m_profileUpdater = new std::thread([this] {
        double height;
        while (m_updateProfile) {
            if (!atGoal()) {
                height = updateSetpoint(m_profileTimer->Get());
            }
            else {
                height = m_setpoint;
            }
            setHeight(height);
            std::this_thread::sleep_for(std::chrono::milliseconds(
                                            10));
        }
    });

    // Load motion profile constants from the configuration file
    m_maxv_a = m_settings->getDouble("EV_MAX_VELOCITY_PROFILE_A");
    m_ttmaxv_a = m_settings->getDouble("EV_TIME_TO_MAX_VELOCITY_PROFILE_A");
    m_maxv_b = m_settings->getDouble("EV_MAX_VELOCITY_PROFILE_B");
    m_ttmaxv_b = m_settings->getDouble("EV_TIME_TO_MAX_VELOCITY_PROFILE_B");

    // Load elevator levels from the configuration file
    m_maxHeight = m_settings->getDouble("EV_MAX_HEIGHT");
    m_toteHeights["EV_GROUND"] = m_settings->getDouble("EV_GROUND");

    double height = 0;
    for (int i = 1; i <= 6; i++) {
        height = m_settings->getDouble("EV_TOTE_" + std::to_string(i));
        m_toteHeights["EV_TOTE_" + std::to_string(i)] = height;
    }

    m_toteHeights["EV_STEP"] = m_settings->getDouble("EV_STEP");
    m_toteHeights["EV_HALF_TOTE_OFFSET"] = m_settings->getDouble(
        "EV_HALF_TOTE_OFFSET");
    m_toteHeights["EV_GARBAGECAN_LEVEL"] = m_settings->getDouble(
        "EV_GARBAGECAN_LEVEL");
    m_toteHeights["EV_AUTO_DROP_LENGTH"] = m_settings->getDouble(
        "EV_AUTO_DROP_LENGTH");

    State* state = new State("IDLE");
    state->initFunc = [this] { m_startAutoStacking = false; };
    state->advanceFunc = [this] {
        if (m_startAutoStacking) {
            return "WAIT_INITIAL_HEIGHT";
        }
        else {
            return "";
        }
    };
    m_autoStackSM.addState(state);
    m_autoStackSM.setState("IDLE");

    state = new State("WAIT_INITIAL_HEIGHT");
    state->initFunc = [this] {
        setProfileHeight(m_toteHeights["EV_TOTE_1"]);
    };
    state->advanceFunc = [this] {
        if (atGoal()) {
            return "SEEK_DROP_TOTES";
        }
        else {
            return "";
        }
    };
    m_autoStackSM.addState(state);

    state = new State("SEEK_DROP_TOTES");
    state->initFunc = [this] {
        setProfileHeight(getGoal() - m_toteHeights["EV_AUTO_DROP_LENGTH"]);
    };
    state->advanceFunc = [this] {
        if (atGoal()) {
            return "RELEASE";
        }
        else {
            return "";
        }
    };
    m_autoStackSM.addState(state);

    state = new State("RELEASE");
    state->initFunc = [this] {
        m_grabTimer->Reset();
        m_grabTimer->Start();
        elevatorGrab(false);
    };
    state->advanceFunc = [this] {
        if (m_grabTimer->HasPeriodPassed(0.2)) {
            return "SEEK_GROUND";
        }
        else {
            return "";
        }
    };
    m_autoStackSM.addState(state);

    state = new State("SEEK_GROUND");
    state->initFunc = [this] {
        setProfileHeight(m_toteHeights["EV_GROUND"]);
    };
    state->advanceFunc = [this] {
        if (atGoal()) {
            return "GRAB";
        }
        else {
            return "";
        }
    };
    m_autoStackSM.addState(state);

    state = new State("GRAB");
    state->initFunc = [this] {
        m_grabTimer->Reset();
        m_grabTimer->Start();
        elevatorGrab(true);
    };
    state->advanceFunc = [this] {
        if (m_grabTimer->HasPeriodPassed(0.4)) {
            return "SEEK_HALF_TOTE";
        }
        else {
            return "";
        }
    };
    m_autoStackSM.addState(state);

    state = new State("SEEK_HALF_TOTE");
    state->initFunc = [this] {
        setProfileHeight(m_toteHeights["EV_TOTE_1"]);
    };
    state->advanceFunc = [this] {
        if (atGoal()) {
            return "INTAKE_IN";
        }
        else {
            return "";
        }
    };
    m_autoStackSM.addState(state);

    state = new State("INTAKE_IN");
    state->initFunc = [this] {
        m_grabTimer->Reset();
        m_grabTimer->Start();
        intakeGrab(true);
    };
    state->advanceFunc = [this] {
        if (m_grabTimer->HasPeriodPassed(0.2)) {
            return "IDLE";
        }
        else {
            return "";
        }
    };
    m_autoStackSM.addState(state);

    // Reload PID constants from the configuration file
    reloadPID();
}

Elevator::~Elevator() {
    m_updateProfile = false;
    if (m_profileUpdater != nullptr) {
        m_profileUpdater->join();
        delete m_profileUpdater;
    }
}

void Elevator::elevatorGrab(bool state) {
    m_elevatorGrabber->Set(!state);
}

bool Elevator::isElevatorGrabbed() const {
    return !m_elevatorGrabber->Get();
}

void Elevator::intakeGrab(bool state) {
    m_intakeGrabber->Set(state);
}

bool Elevator::isIntakeGrabbed() const {
    return m_intakeGrabber->Get();
}

void Elevator::stowIntake(bool state) {
    m_intakeStower->Set(!state);
}

bool Elevator::isIntakeStowed() const {
    return !m_intakeStower->Get();
}

void Elevator::containerGrab(bool state) {
    m_containerGrabber->Set(!state);
}

bool Elevator::isContainerGrabbed() const {
    return !m_containerGrabber->Get();
}

void Elevator::setIntakeDirectionLeft(IntakeMotorState state) {
    m_intakeState = state;

    if (state == S_STOPPED) {
        m_intakeWheelLeft->Set(0);
    }
    else if (state == S_FORWARD) {
        m_intakeWheelLeft->Set(1);
    }
    else if (state == S_REVERSE) {
        m_intakeWheelLeft->Set(-1);
    }
    else if (state == S_ROTATE_CCW) {
        m_intakeWheelLeft->Set(-1);
    }
    else if (state == S_ROTATE_CW) {
        m_intakeWheelLeft->Set(1);
    }
}

void Elevator::setIntakeDirectionRight(IntakeMotorState state) {
    m_intakeState = state;

    if (state == S_STOPPED) {
        m_intakeWheelRight->Set(0);
    }
    else if (state == S_FORWARD) {
        m_intakeWheelRight->Set(-1);
    }
    else if (state == S_REVERSE) {
        m_intakeWheelRight->Set(1);
    }
    else if (state == S_ROTATE_CCW) {
        m_intakeWheelRight->Set(-1);
    }
    else if (state == S_ROTATE_CW) {
        m_intakeWheelRight->Set(1);
    }
}

Elevator::IntakeMotorState Elevator::getIntakeDirection() const {
    return m_intakeState;
}

void Elevator::setManualLiftSpeed(double value) {
    if (m_manual) {
        m_liftGrbx->setManual(value);
    }
}

double Elevator::getManualLiftSpeed() const {
    if (m_manual) {
        return m_liftGrbx->get(Grbx::Raw);
    }

    return 0.0;
}

void Elevator::setManualMode(bool on) {
    if (!on && m_manual) {
        m_manual = false;
        setProfileHeight(getHeight());
    }
    else if (on && !m_manual) {
        m_manual = true;

        // Stop any auto-stacking when we switch to manual mode
        m_autoStackSM.setState("IDLE");
    }
}

bool Elevator::isManualMode() const {
    return m_manual;
}

void Elevator::setHeight(double height) {
    if (m_manual == false) {
        m_liftGrbx->setSetpoint(height);
    }
}

double Elevator::getHeight() const {
    return m_liftGrbx->get(Grbx::Position);
}

void Elevator::reloadPID() {
    m_settings->update();

    // First profile
    double p = 0.f;
    double i = 0.f;
    double d = 0.f;
    double f = 0.f;

    // Set elevator PID
    p = m_settings->getDouble("PID_ELEVATOR_DOWN_P");
    i = m_settings->getDouble("PID_ELEVATOR_DOWN_I");
    d = m_settings->getDouble("PID_ELEVATOR_DOWN_D");
    f = m_settings->getDouble("PID_ELEVATOR_DOWN_F");

    m_liftGrbx->setProfile(false);
    m_liftGrbx->setPID(p, i, d);
    m_liftGrbx->setF(f);

    // Set elevator PID
    p = m_settings->getDouble("PID_ELEVATOR_UP_P");
    i = m_settings->getDouble("PID_ELEVATOR_UP_I");
    d = m_settings->getDouble("PID_ELEVATOR_UP_D");
    f = m_settings->getDouble("PID_ELEVATOR_UP_F");

    m_liftGrbx->setProfile(true);
    m_liftGrbx->setPID(p, i, d);
    m_liftGrbx->setF(f);
}

void Elevator::resetEncoders() {
    m_liftGrbx->resetEncoder();
}

void Elevator::pollLiftLimitSwitches() {
    // Check encoder reset limit switch
    if (m_liftGrbx->isRevLimitSwitchClosed()) {
        m_liftGrbx->resetEncoder();
    }
}

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
        }
        else {
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
            }
            else if (level[op] == '-') {
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
        m_liftGrbx->setProfile(true);
    }
    else {
        // Going down.
        setMaxVelocity(91.26);
        setTimeToMaxV(0.4);
        m_liftGrbx->setProfile(false);
    }

    m_profileTimer->Reset();
    m_profileTimer->Start();
    setGoal(m_profileTimer->Get(), height, getHeight());
}

double Elevator::getLevelHeight(std::string level) const {
    auto height = m_toteHeights.find(level);

    if (height == m_toteHeights.end()) {
        return 0.0;
    }
    else {
        return height->second;
    }
}

void Elevator::stackTotes() {
    setManualMode(false);
    m_startAutoStacking = true;
}

bool Elevator::isStacking() const {
    return m_autoStackSM.getState() != "IDLE";
}

void Elevator::cancelStack() {
    m_autoStackSM.setState("IDLE");
}

void Elevator::updateState() {
    m_autoStackSM.run();
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
        m_liftGrbx->setProfile(true);
    }
    else {
        // Going down.
        setMaxVelocity(m_maxv_b);
        setTimeToMaxV(m_ttmaxv_b);
        m_liftGrbx->setProfile(false);
    }

    m_setpoint = newSetpoint;
}

