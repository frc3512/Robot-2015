/*
 * LogConsoleSink.h
 *
 *  Created on: Nov 10, 2013
 *      Author: acf
 */

#ifndef LOGCONSOLESINK_H_
#define LOGCONSOLESINK_H_

#include "LogSinkBase.h"
#include <iostream>

/*!
  A sink for writing logged events to standard output.
 */
class LogConsoleSink: public LogSinkBase
{
public:
    LogConsoleSink();
    virtual ~LogConsoleSink();

    /*!
      Write an event to standard output.
      \param event The event to log.
     */
    void logEvent(LogEvent event);
};

#endif /* LOGCONSOLESINK_H_ */
