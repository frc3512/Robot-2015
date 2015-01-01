/*
 * LogConsoleSink.cpp
 *
 *  Created on: Nov 10, 2013
 *      Author: acf
 */

#include "LogConsoleSink.h"
#include <iostream>

LogConsoleSink::LogConsoleSink()
{
    // TODO Auto-generated constructor stub

}

LogConsoleSink::~LogConsoleSink()
{
    // TODO Auto-generated destructor stub
}

void LogConsoleSink::logEvent(LogEvent event)
{
    std::cout << event.toFormattedString();
}
