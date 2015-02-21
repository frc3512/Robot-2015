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
    m_grabSolenoid = std::make_unique<Solenoid>(3);

    m_intakeVertical = std::make_unique<Solenoid>(1);
    m_intakeGrabber = std::make_unique<Solenoid>(2);
    m_intakeWheelLeft = std::make_unique<CANTalon>(3);
    m_intakeWheelRight = std::make_unique<CANTalon>(6);
    m_frontLeftLimit = std::make_unique<DigitalInput>(0);
    m_frontRightLimit = std::make_unique<DigitalInput>(1);
    m_settings = std::make_unique<Settings>("/home/lvuser/RobotSettings.txt");
    m_intakeState = S_STOPPED;
    m_manual = false;

#if 0
    // For WPILib PID loop
    m_liftGrbx = std::make_unique<GearBox<CANTalon>>(-1, 2, 3, 2, 7);

    /* gear ratio is 48 driver to 26 driven from output of gearbox (where
     * encoder shaft is located), therefore:
     * distance per pulse = 26/48/(number of pulses per revolution)
     *                    = 26/48/360
     *                    = 26/(48*360)
     */
    m_liftGrbx->setDistancePerPulse((70.5 / 1.92442) * (26.0 / (48.0 * 360.0)));
#else
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
#endif

    m_profileTimer = std::make_unique<Timer>();
    m_grabTimer = std::make_unique<Timer>();
    m_updateProfile = true;
    m_profileUpdater = nullptr;

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

    m_maxv_a = m_settings->getDouble("EV_MAX_VELOCITY_PROFILE_A");
    m_ttmaxv_a = m_settings->getDouble("EV_TIME_TO_MAX_VELOCITY_PROFILE_A");
    m_maxv_b = m_settings->getDouble("EV_MAX_VELOCITY_PROFILE_B");
    m_ttmaxv_b = m_settings->getDouble("EV_TIME_TO_MAX_VELOCITY_PROFILE_B");

    m_maxHeight = m_settings->getDouble("EV_MAX_HEIGHT");
    m_toteHeights["EV_GROUND"] = m_settings->getDouble("EV_GROUND");

    double height = 0;
    for (int i = 0; i <= 6; i++) {
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

    State* state = new State("WAIT_INITIAL_HEIGHT");
    state->initFunc = [this] { setProfileHeight(getGoal() - 5.0); };
    state->advanceFunc = [this] {
        if (atGoal()) {
            return "SEEK_DROP_TOTES";
        }
        else {
            return "";
        }
    };
    m_autoStackSM.addState(state);
    m_autoStackSM.setInitialState("WAIT_INITIAL_HEIGHT");

    state = new State("SEEK_DROP_TOTES");
    state->initFunc = [this] { setProfileHeight(getGoal() - 5.0); };
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
    state->initFunc = [this] { setProfileHeight(m_toteHeights["EV_GROUND"]); };
    state->periodicFunc = [this] { return atGoal(); };
    m_autoStackSM.addState(state);

    state = new State("GRAB");
    state->initFunc = [this] {
        m_grabTimer->Reset();
        m_grabTimer->Start();
        elevatorGrab(true);
    };
    state->advanceFunc = [this] {
        if (m_grabTimer->HasPeriodPassed(0.2)) {
            return "SEEK_HALF_TOTE";
        }
        else {
            return "";
        }
    };
    m_autoStackSM.addState(state);

    state = new State("SEEK_HALF_TOTE");
    state->initFunc = [this] { setProfileHeight(m_toteHeights["EV_TOTE_1"]); };
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
    m_grabSolenoid->Set(!state);
}

bool Elevator::isElevatorGrabbed() {
    return !m_grabSolenoid->Get();
}

void Elevator::intakeGrab(bool state) {
    m_intakeGrabber->Set(state);
}

bool Elevator::isIntakeGrabbed() {
    return m_intakeGrabber->Get();
}

void Elevator::stowIntake(bool state) {
    m_intakeVertical->Set(!state);
}

bool Elevator::isIntakeStowed() {
    return !m_intakeVertical->Get();
}

void Elevator::setIntakeDirection(IntakeMotorState state) {
    m_intakeState = state;

    if (state == S_STOPPED) {
        m_intakeWheelLeft->Set(0);
        m_intakeWheelRight->Set(0);
    }
    else if (state == S_FORWARD) {
        m_intakeWheelLeft->Set(1);
        m_intakeWheelRight->Set(-1);
    }
    else if (state == S_REVERSED) {
        m_intakeWheelLeft->Set(-1);
        m_intakeWheelRight->Set(1);
    }
}

Elevator::IntakeMotorState Elevator::getIntakeDirection() {
    return m_intakeState;
}

void Elevator::setManualLiftSpeed(double value) {
    if (m_manual) {
        m_liftGrbx->setManual(value);
    }
}

double Elevator::getManualLiftSpeed() {
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
    else {
        m_manual = on;
    }

    // Stop any auto stacking we're working on when we switch to manual mode
    if (on && !m_manual) {
        m_autoStackSM.cancel();
    }
}

bool Elevator::isManualMode() {
    return m_manual;
}

void Elevator::setHeight(double height) {
    if (m_manual == false) {
        m_liftGrbx->setSetpoint(height);
    }
}

double Elevator::getHeight() {
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

void Elevator::resetEncoder() {
    m_liftGrbx->resetEncoder();
}

void Elevator::pollLimitSwitches() {
    // Check encoder reset limit switch
    if (m_liftGrbx->isRevLimitSwitchClosed()) {
        m_liftGrbx->resetEncoder();
    }

    // Check front limit switches
    if (!m_frontLeftLimit->Get() && !m_frontRightLimit->Get()) {
        stackTotes();
    }
}

void Elevator::raiseElevator(std::string level) {
    size_t pos;
    size_t newpos;
    double height = 0;

    pos = level.find("+");
    auto it = m_toteHeights.find(level.substr(0, pos));
    if (it != m_toteHeights.end()) {
        height = it->second;
    }
    while (pos != std::string::npos) {
        newpos = level.find("+", pos + 1);
        it = m_toteHeights.find(level.substr(pos + 1, newpos));
        if (it != m_toteHeights.end()) {
            height += it->second;
        }
        pos = newpos;
    }

    /* Only allow changing the elevator height manually if not currently
     * auto-stacking
     */
    if (m_autoStackSM.isStopped()) {
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
    m_autoStackSM.start();
}

bool Elevator::isStacking() {
    return m_autoStackSM.isStopped();
}

void Elevator::cancelStack() {
    m_autoStackSM.cancel();
}

void Elevator::updateState() {
    m_autoStackSM.run();
}

void Elevator::manualChangeSetpoint(double delta) {
    double newSetpoint = delta + m_setpoint;

    if (newSetpoint > m_maxHeight) {
        newSetpoint = m_maxHeight;
    }

    // TODO: Magic numbers
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

