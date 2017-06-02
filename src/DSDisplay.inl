// Copyright (c) 2015-2017 FRC Team 3512. All Rights Reserved.

#pragma once

#include <string>

template <class T>
void DSDisplay::AddAutoMethod(const std::string& methodName,
                              void (T::*function)(), T* object) {
    m_autonModes.addMethod(methodName, std::bind(function, object));
}
