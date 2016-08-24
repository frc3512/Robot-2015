// Copyright (c) FRC Team 3512, Spartatroniks 2015-2016. All Rights Reserved.

#include "State.hpp"

State::State(std::string name) : m_name{std::move(name)} {}

const std::string& State::Name() const { return m_name; }
