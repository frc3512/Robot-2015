/*
 * Logger.cpp
 *
 *  Created on: Nov 10, 2013
 *      Author: acf
 */

#include "Logger.h"

Logger::Logger()
{
    resetInitialTime();
}

Logger::~Logger()
{
    // TODO Auto-generated destructor stub
}

void Logger::logEvent(LogEvent event)
{
    LogSinkBaseList::iterator it;

    event.setInitialTime(m_initialTime);

    for(it = m_sinkList.begin(); it != m_sinkList.end(); it++) {
        if((*it)->testVerbosityLevel(event.getVerbosityLevel()))
            (*it)->logEvent(event);
    }
}

void Logger::addLogSink(LogSinkBase *sink)
{
    m_sinkList.push_back(sink);
}

void Logger::removeLogSink(LogSinkBase *sink)
{
    m_sinkList.remove(sink);
}

Logger::LogSinkBaseList Logger::listLogSinks()
{
    return m_sinkList;
}

void Logger::resetInitialTime()
{
    m_initialTime = time(nullptr);
}

void Logger::setInitialTime(time_t time)
{
    m_initialTime = time;
}
