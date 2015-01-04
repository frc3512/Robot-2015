/*
 * LogFileSink.cpp
 *
 *  Created on: Nov 10, 2013
 *      Author: acf
 */

#include "LogFileSink.h"

LogFileSink::LogFileSink(std::string filename)
{
    m_logfile.open(filename.c_str());
}

LogFileSink::~LogFileSink()
{
    // TODO Auto-generated destructor stub
}

void LogFileSink::logEvent(LogEvent event)
{
    m_logfile << event.toFormattedString();
    m_logfile.flush();

}
