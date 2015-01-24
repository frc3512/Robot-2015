// =============================================================================
// File Name: AutonContainer.hpp
// Description: Stores Autonomous modes as function pointers for easy retrieval
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#ifndef AUTON_CONTAINER_HPP
#define AUTON_CONTAINER_HPP

#include <vector>
#include <string>
#include <functional>

struct AutonMethod {
    std::string name;
    std::function<void()> function;

    AutonMethod( const std::string& methodName , std::function<void()> func ) {
        name = methodName;
        function = func;
    }
};

class AutonContainer {
public:
    virtual ~AutonContainer();

    // Add and remove autonomous functions
    void addMethod( const std::string& methodName ,
                    std::function<void()> func );
    void deleteAllMethods();

    // Returns number of routines currently held
    size_t size();

    // Returns name of specific autonomous function
    const std::string& name( size_t pos );

    // Run specific autonomous function
    void execAutonomous( size_t pos );

private:
    // Contains function pointers to the autonomous functions
    std::vector<AutonMethod> m_functionList;
};

#endif // AUTON_CONTAINER_HPP

