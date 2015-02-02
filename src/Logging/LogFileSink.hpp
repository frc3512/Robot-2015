/*
 * LogFileSink.h
 *
 *  Created on: Nov 10, 2013
 *      Author: acf
 */

#ifndef LOGFILESINK_H_
#define LOGFILESINK_H_

#include "LogSinkBase.hpp"
#include <fstream>

/*!
 *  A file sink for the logged events.
 */
class LogFileSink : public LogSinkBase {
public:
    LogFileSink(std::string filename);
    virtual ~LogFileSink();

    /*!
     *  Write an event to the logfile.
     *  \param event The event to log.
     */
    void logEvent(LogEvent event);

private:
    std::ofstream m_logfile;
};

#endif /* LOGFILESINK_H_ */

