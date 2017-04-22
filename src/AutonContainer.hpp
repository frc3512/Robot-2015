// Copyright (c) FRC Team 3512, Spartatroniks 2015-2017. All Rights Reserved.

#pragma once

#include <functional>
#include <string>
#include <vector>

struct AutonMethod {
    std::string name;
    std::function<void()> function;

    AutonMethod(const std::string& methodName, std::function<void()> func);
};

/**
 * Stores Autonomous modes as function pointers for easy retrieval
 */
class AutonContainer {
public:
    // Add and remove autonomous functions
    void addMethod(const std::string& methodName, std::function<void()> func);
    void deleteAllMethods();

    // Returns number of routines currently held
    size_t size();

    // Returns name of specific autonomous function
    const std::string& name(size_t pos);

    // Run specific autonomous function
    void execAutonomous(size_t pos);

private:
    // Contains function pointers to the autonomous functions
    std::vector<AutonMethod> m_functionList;
};
