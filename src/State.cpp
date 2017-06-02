// Copyright (c) 2015-2017 FRC Team 3512. All Rights Reserved.

#include "State.hpp"

#include <utility>

State::State(std::string name) : m_name{std::move(name)} {}

const std::string& State::Name() const { return m_name; }
