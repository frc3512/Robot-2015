/*
 * LogStreambuf.cpp
 *
 *  Created on: Nov 11, 2013
 *      Author: acf
 */

#include "LogStreambuf.h"

LogStreambuf::LogStreambuf(Logger *logger)
{
    // TODO Auto-generated constructor stub
    m_logger = logger;
    m_level = LogEvent::VERBOSE_NONE;

}

LogStreambuf::~LogStreambuf()
{
    // TODO Auto-generated destructor stub
}

void LogStreambuf::setLevel(LogEvent::VerbosityLevel level)
{
    m_level = level;

    return;
}

int LogStreambuf::sync()
{
    if(m_level != LogEvent::VERBOSE_NONE)
        m_logger->logEvent(LogEvent(m_buf, m_level));

    m_buf = "";
    m_level = LogEvent::VERBOSE_NONE;

    return 0;
}

std::streamsize LogStreambuf::xsputn(const char *s, std::streamsize n)
{
    m_buf += std::string(s, n);

    return n;
}

