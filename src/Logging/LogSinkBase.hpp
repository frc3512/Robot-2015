/*
 * LogSinkBase.h
 *
 *  Created on: Nov 10, 2013
 *      Author: acf
 */

#ifndef LOGSINKBASE_H_
#define LOGSINKBASE_H_

#include "LogEvent.hpp"

/*!
 *  LogSinkBase provides a base class on which to implement
 *  various log event sinks. A log event sink is a class
 *  which receives log events (in the form of LogEvent
 *  class instances) though the logEvent() callback function.
 *  The class can then store, display, or otherwise process
 *  the event.
 */
class LogSinkBase {
public:
    /*!
     *  The constructor.
     */
    LogSinkBase();
    virtual ~LogSinkBase();

    /*!
     *  The callback function called when an event
     *  is received whose verbosity level matches
     *  that of the class (see testVerbosityLevel).
     *  \param event The event
     */
    virtual void logEvent( LogEvent event ) = 0;

    /*!
     *  Set the verbosity levels for which we will
     *  accept events.
     *  \param levels A bitfield describing the verbosity
     *  levels.
     */
    void setVerbosityLevels( LogEvent::VerbosityLevel levels );

    /*!
     *  Get the verbosity levels for which we are
     *  currently accepting events for
     *  \return A bitfield describing the verbosity levels.
     */
    LogEvent::VerbosityLevel getVerbosityLevels();

    /*!
     *  Begin accepting events of the specified verbosity
     *  levels.
     *  \param levels A bitfield describing the verbosity
     *  levels.
     */
    void enableVerbosityLevels( LogEvent::VerbosityLevel levels );

    /*!
     *  Stop accepting events of the specified verbosity
     *  levels.
     *  \param levels A bitfield describing the verbosity
     *  levels.
     */
    void disableVerbosityLevels( LogEvent::VerbosityLevel levels );

    /*!
     *  Determines whether or not we accept events of a
     *  specified verbosity level.
     *  \param levels A bitfield describing the verbosity
     *  levels to test.
     *  \return A boolean describing whether we accept
     *  events of the specified verbosity level.
     */
    bool testVerbosityLevel( LogEvent::VerbosityLevel levels );

private:
    LogEvent::VerbosityLevel m_verbosity;
};

#endif /* LOGSINKBASE_H_ */

