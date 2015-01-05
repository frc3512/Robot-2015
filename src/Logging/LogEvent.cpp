/*
 * LogEvent.cpp
 *
 *  Created on: Nov 10, 2013
 *      Author: acf
 */

#include "LogEvent.hpp"
#include <sstream>
#include <iomanip>

LogEvent::LogEvent( std::string data , VerbosityLevel level ) {
    m_level = level;
    m_timestamp = time( nullptr );
    m_buffer = data;
    m_initialTime = 0;
}

LogEvent::LogEvent( std::string data , VerbosityLevel level ,
                    time_t timestamp ) {
    m_level = level;
    m_timestamp = timestamp;
    m_buffer = data;
    m_initialTime = 0;
}

LogEvent::~LogEvent() {
    // TODO Auto-generated destructor stub
}

LogEvent::VerbosityLevel LogEvent::getVerbosityLevel() {
    return m_level;
}

time_t LogEvent::getAbsoluteTimestamp() {
    return m_timestamp;
}

time_t LogEvent::getRelativeTimestamp() {
    if ( m_initialTime == 0 ) {
        return 0;
    }

    return m_timestamp - m_initialTime;
}

std::string LogEvent::getData() {
    return m_buffer;
}

std::string LogEvent::toFormattedString() {
    std::stringstream ss;

    ss << std::left << "[" << std::setw( 8 )
       << getRelativeTimestamp() << " "
       << verbosityLevelChar( getVerbosityLevel() )
       << "] " << getData() << "\n";

    return ss.str();
}

std::string LogEvent::verbosityLevelString( VerbosityLevel levels ) {
    std::string level_str;

    /* if(level == VERBOSE_ALL)
     *   return "VERBOSE_ALL"; */

    if ( levels & VERBOSE_ERROR ) {
        level_str += "ERROR ";
    }

    if ( levels & VERBOSE_WARN ) {
        level_str += "WARN ";
    }

    if ( levels & VERBOSE_INFO ) {
        level_str += "INFO ";
    }

    if ( levels & VERBOSE_DEBUG ) {
        level_str += "DEBUG ";
    }

    if ( levels & VERBOSE_USER ) {
        level_str += "USER ";
    }

    return level_str;

    /*
     *  switch(level) {
     *  case VERBOSE_ERROR:
     *   return "VERBOSE_ERROR";
     *  case VERBOSE_WARN:
     *   return "VERBOSE_WARN";
     *  case VERBOSE_INFO:
     *   return "VERBOSE_INFO";
     *  case VERBOSE_DEBUG:
     *   return "VERBOSE_DEBUG";
     *  case VERBOSE_USER:
     *   return "VERBOSE_USER";
     *  case VERBOSE_NONE:
     *  case VERBOSE_ALL:
     *  default:
     *   return "UNKNOWN";
     *  }
     */
}

char LogEvent::verbosityLevelChar( VerbosityLevel level ) {
    switch ( level ) {
    case VERBOSE_ERROR:
        return 'E';
    case VERBOSE_WARN:
        return 'W';
    case VERBOSE_INFO:
        return 'I';
    case VERBOSE_DEBUG:
        return 'D';
    case VERBOSE_USER:
        return 'U';
    case VERBOSE_NONE:
    case VERBOSE_ALL:
    default:
        return 'X';
    }
}

void LogEvent::setInitialTime( time_t initial ) {
    m_initialTime = initial;
}

