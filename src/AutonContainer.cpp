// =============================================================================
// File Name: AutonContainer.cpp
// Description: Stores Autonomous modes as function pointers for easy retrieval
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "AutonContainer.hpp"

AutonMethod::AutonMethod(const std::string& methodName,
        std::function<void()> func) {
    name = methodName;
    function = func;
}

void AutonContainer::addMethod(const std::string& methodName,
                               std::function<void()> func) {
    m_functionList.push_back(AutonMethod(methodName, func));
}

void AutonContainer::deleteAllMethods() {
    m_functionList.clear();
}

size_t AutonContainer::size() {
    return m_functionList.size();
}

const std::string& AutonContainer::name(size_t pos) {
    return m_functionList[pos].name;
}

void AutonContainer::execAutonomous(size_t pos) {
    // Retrieves correct autonomous routine
    auto& auton = m_functionList[pos];

    // Runs the routine
    (auton.function)();
}


