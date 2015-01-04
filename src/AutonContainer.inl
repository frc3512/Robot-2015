//=============================================================================
//File Name: AutonContainer.inl
//Description: Stores Autonomous modes as function pointers for easy retrieval
//Author: FRC Team 3512, Spartatroniks
//=============================================================================

template <class T>
AutonContainer<T>::~AutonContainer() {
    deleteAllMethods();
}

template <class T>
void AutonContainer<T>::addMethod( const std::string& methodName , void (T::*function)() , T* object ) {
    m_functionList.push_back( AutonMethod<T>( methodName , function , object ) );
}

template <class T>
void AutonContainer<T>::deleteAllMethods() {
    m_functionList.clear();
}

template <class T>
size_t AutonContainer<T>::size() {
    return m_functionList.size();
}

template <class T>
const std::string& AutonContainer<T>::name( size_t pos ) {
    return m_functionList[pos].name;
}

template <class T>
void AutonContainer<T>::execAutonomous( size_t pos ) {
    // Retrieves correct autonomous routine
    AutonMethod<T>* auton = &m_functionList[pos];

    // Runs the routine
    ((auton->object)->*(auton->function))();
}
