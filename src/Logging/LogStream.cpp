/*
 * LogStream.cpp
 *
 *  Created on: Nov 11, 2013
 *      Author: acf
 */

#include "LogStream.h"

LogStream::LogStream( Logger* logger ) :   std::ostream( new LogStreambuf(
                                                             logger ) ) {
    m_logger = logger;
}

LogStream::~LogStream() {
    delete rdbuf();
    // TODO Auto-generated destructor stub
}

void LogStream::setLevel( LogEvent::VerbosityLevel level ) {
    ( (LogStreambuf*) rdbuf() )->setLevel( level );
}

