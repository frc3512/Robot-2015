//=============================================================================
//File Name: AutonContainer.hpp
//Description: Stores Autonomous modes as function pointers for easy retrieval
//Author: FRC Team 3512, Spartatroniks
//=============================================================================

#ifndef AUTON_CONTAINER_HPP
#define AUTON_CONTAINER_HPP

#include <vector>
#include <string>

template <class T>
struct AutonMethod {
    std::string name;
    void (T::*function)();
    T* object;

    AutonMethod( const std::string& methodName , void (T::*func)() , T* obj ) {
        name = methodName;
        function = func;
        object = obj;
    }
};

template <class T>
class AutonContainer {
public:
    virtual ~AutonContainer();

    // Add and remove autonomous functions
    void addMethod( const std::string& methodName , void (T::*function)() , T* object );
    void deleteAllMethods();

    // Returns number of routines currently held
    size_t size();

    // Returns name of specific autonomous function
    const std::string& name( size_t pos );

    // Run specific autonomous function
    void execAutonomous( size_t pos );

private:
    // Contains function pointers to the autonomous functions
    std::vector<AutonMethod<T> > m_functionList;
};

#include "AutonContainer.inl"

#endif // AUTON_CONTAINER_HPP
