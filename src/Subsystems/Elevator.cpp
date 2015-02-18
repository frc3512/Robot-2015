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

    m_state = STATE_IDLE;

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
	if(m_manual) {
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
    double p0 = 0.f;
    double i0 = 0.f;
    double d0 = 0.f;
    double f0 = 0.f;

    // Set elevator PID
    p0 = m_settings->getDouble("PID_ELEVATOR_DOWN_P");
    i0 = m_settings->getDouble("PID_ELEVATOR_DOWN_I");
    d0 = m_settings->getDouble("PID_ELEVATOR_DOWN_D");
    f0 = m_settings->getDouble("PID_ELEVATOR_DOWN_F");

    m_liftGrbx->setProfile(false);
    m_liftGrbx->setPID(p0, i0, d0);
    m_liftGrbx->setF(f0);

    // Second profile
    double p1 = 0.f;
    double i1 = 0.f;
    double d1 = 0.f;
    double f1 = 0.f;

    // Set elevator PID
    p1 = m_settings->getDouble("PID_ELEVATOR_UP_P");
    i1 = m_settings->getDouble("PID_ELEVATOR_UP_I");
    d1 = m_settings->getDouble("PID_ELEVATOR_UP_D");
    f1 = m_settings->getDouble("PID_ELEVATOR_UP_F");

    m_liftGrbx->setProfile(true);
    m_liftGrbx->setPID(p1, i1, d1);
    m_liftGrbx->setF(f1);
}

void Elevator::resetEncoder() {
    resetEncoder(0, m_liftGrbx.get());
}

bool Elevator::isStacking() {
	return m_state != STATE_IDLE;
}

void Elevator::resetEncoder(uint32_t interruptAssertedMask, void* param) {
    reinterpret_cast<decltype(m_liftGrbx.get())>(param)->resetEncoder();
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
    if (m_state == STATE_IDLE) {
        std::cout << "Seeking to " << height << std::endl;

        setProfileHeight(height);
    }
}

void Elevator::setProfileHeight(double height) {
    // Don't try to seek anywhere if we're already at setpoint
    /* if (height == getHeight()) {
     *   return;
     *  } */

    // TODO: magic number
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
    if (m_state == STATE_IDLE) {
        m_state = STATE_WAIT_INITIAL_HEIGHT;
        stateChanged(STATE_IDLE, m_state);
    }
}

void Elevator::updateState() {
    if (m_state == STATE_WAIT_INITIAL_HEIGHT && atGoal()) {
        m_state = STATE_SEEK_DROP_TOTES;
        stateChanged(STATE_WAIT_INITIAL_HEIGHT, m_state);
    }
    if (m_state == STATE_SEEK_DROP_TOTES && atGoal()) {
        m_state = STATE_RELEASE;
        stateChanged(STATE_SEEK_DROP_TOTES, m_state);
    }
    else if (m_state == STATE_RELEASE && m_grabTimer->HasPeriodPassed(0.2)) {
        m_state = STATE_SEEK_GROUND;
        stateChanged(STATE_RELEASE, m_state);
    }
    else if (m_state == STATE_SEEK_GROUND && atGoal()) {
        m_state = STATE_GRAB;
        stateChanged(STATE_SEEK_GROUND, m_state);
    }
    else if (m_state == STATE_GRAB && m_grabTimer->HasPeriodPassed(0.2)) {
        m_state = STATE_SEEK_HALF_TOTE;
        stateChanged(STATE_GRAB, m_state);
    }
    else if (m_state == STATE_SEEK_HALF_TOTE && atGoal()) {
        m_state = STATE_INTAKE_IN;
        stateChanged(STATE_SEEK_HALF_TOTE, m_state);
    }
    else if (m_state == STATE_INTAKE_IN && m_grabTimer->HasPeriodPassed(0.2)) {
        m_state = STATE_IDLE;
        stateChanged(STATE_INTAKE_IN, m_state);
    }
    else if (m_state != STATE_IDLE && isManualMode()) {
        // FIXME Hack
        m_state = STATE_IDLE;
    }
}

std::string Elevator::to_string(ElevatorState state) {
    switch (state) {
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
    case STATE_INTAKE_IN:
        return "STATE_INTAKE_IN";
    }

    return "UNKNOWN STATE";
}

void Elevator::stateChanged(ElevatorState oldState, ElevatorState newState) {
    std::cout << "oldState = " << to_string(oldState)
              << " newState = " << to_string(newState) << std::endl;

    if (newState == STATE_SEEK_DROP_TOTES) {
        // TODO: magic number
        setProfileHeight(getGoal() - 5.0);
    }

    // Release the totes
    if (newState == STATE_RELEASE) {
        m_grabTimer->Reset();
        m_grabTimer->Start();
        elevatorGrab(false);
    }

    if (newState == STATE_SEEK_GROUND) {
        setProfileHeight(m_toteHeights["EV_GROUND"]);
    }

    // Grab the new stack
    if (newState == STATE_GRAB) {
        m_grabTimer->Reset();
        m_grabTimer->Start();
        elevatorGrab(true);
    }

    // Off the ground a bit
    if (newState == STATE_SEEK_HALF_TOTE) {
        setProfileHeight(m_toteHeights["EV_TOTE_1"]);
    }

    if (newState == STATE_INTAKE_IN) {
        m_grabTimer->Reset();
        m_grabTimer->Start();
        intakeGrab(true);
    }
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

    m_setpoint = newSetpoint;
}

