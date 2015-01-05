/*
 * SetLogLevel.cpp
 *
 *  Created on: Nov 11, 2013
 *      Author: acf
 */

#include "SetLogLevel.h"

SetLogLevel::SetLogLevel( int level ) {
    m_level = level;
}

SetLogLevel::~SetLogLevel() {
    // TODO Auto-generated destructor stub
}

LogStream& operator<<( LogStream& os , const SetLogLevel& in ) {
    os.setLevel( in.m_level );
    return os;
}

