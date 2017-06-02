// Copyright (c) 2015-2017 FRC Team 3512. All Rights Reserved.

#include "AutonContainer.hpp"

AutonMethod::AutonMethod(const std::string& methodName,
                         std::function<void()> func) {
    name = methodName;
    function = func;
}

void AutonContainer::addMethod(const std::string& methodName,
                               std::function<void()> func) {
    m_functionList.emplace_back(methodName, func);
}

void AutonContainer::deleteAllMethods() { m_functionList.clear(); }

size_t AutonContainer::size() { return m_functionList.size(); }

const std::string& AutonContainer::name(size_t pos) {
    return m_functionList[pos].name;
}

void AutonContainer::execAutonomous(size_t pos) {
    // Retrieves correct autonomous routine
    auto& auton = m_functionList[pos];

    // Runs the routine
    (auton.function)();
}
