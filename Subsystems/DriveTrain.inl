//=============================================================================
//File Name: DriveTrain.inl
//Description: Provides an interface for this year's drive train
//Author: FRC Team 3512, Spartatroniks
//=============================================================================

template <class T>
T DriveTrain::limit( T value , T limit ) {
    if ( value > limit ) {
        return limit;
    }
    else if ( value < -limit ) {
        return -limit;
    }
    else {
        return value;
    }
}
