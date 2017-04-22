// Copyright (c) FRC Team 3512, Spartatroniks 2015-2017. All Rights Reserved.

#include "State.hpp"

#include <utility>

State::State(std::string name) : m_name{std::move(name)} {}

const std::string& State::Name() const { return m_name; }
