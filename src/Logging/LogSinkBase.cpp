/*
 * LogSinkBase.cpp
 *
 *  Created on: Nov 10, 2013
 *      Author: acf
 */

#include "LogSinkBase.h"

LogSinkBase::LogSinkBase() {
    /* Default to VERBOSE_ERROR */
    m_verbosity = LogEvent::VERBOSE_ERROR;
}

LogSinkBase::~LogSinkBase() {
}

void LogSinkBase::setVerbosityLevels( LogEvent::VerbosityLevel levels ) {
    m_verbosity = levels;
}

LogEvent::VerbosityLevel LogSinkBase::getVerbosityLevels() {
    return m_verbosity;
}

void LogSinkBase::enableVerbosityLevels( LogEvent::VerbosityLevel levels ) {
    m_verbosity |= levels;
}

void LogSinkBase::disableVerbosityLevels( LogEvent::VerbosityLevel levels ) {
    m_verbosity &= ~( levels );
}

bool LogSinkBase::testVerbosityLevel( LogEvent::VerbosityLevel levels ) {
    return (bool) ( m_verbosity & levels );
}

