/*
 * LogStream.h
 *
 *  Created on: Nov 11, 2013
 *      Author: acf
 */

#ifndef LOGSTREAM_H_
#define LOGSTREAM_H_

#include "LogEvent.hpp"
#include "Logger.hpp"
#include "LogStreambuf.hpp"
#include <iostream>


/*!
  A subclass of std::ostream for logging messages
  with a Logger class. To log a message, one must
  either pipe a SetLogLevel instance into the
  stream or call the setLevel function of the
  class in order to notify the class of the log
  level with which to log the message. After setting
  the verbosity level, one or more strings should
  be piped into the stream, representing the message
  that will be displayed. When the entire message
  description has been piped into the stream, a
  std::flush must be piped into the stream to notify
  the class to log the message. The verbosity level must
  be set each time a new message is logged. If the
  verbosity level is not set, the message will be
  dropped. Currently the current time will always be
  used as the timestamp for the log message.
 */
class LogStream: public std::ostream
{
public:
    /*!
      The constructor.
      \param logger The logger class instance to log
       messages to.
     */
    LogStream(Logger *logger);
    virtual ~LogStream();

    /*!
      Sets the verbosity level with which to log the
      current message.
      \param level The verbosity level with which to
      log the current message.
     */
    void setLevel(LogEvent::VerbosityLevel level);

private:
    Logger *m_logger;
};

#endif /* LOGSTREAM_H_ */
