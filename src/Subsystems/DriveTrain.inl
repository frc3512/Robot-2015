// Copyright (c) FRC Team 3512, Spartatroniks 2015-2016. All Rights Reserved.

#pragma once

template <class T>
T DriveTrain::limit(T value, T limit) {
    if (value > limit) {
        return limit;
    } else if (value < -limit) {
        return -limit;
    } else {
        return value;
    }
}
