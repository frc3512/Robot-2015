/*
 * LogServerSink.h
 *
 *  Created on: Nov 10, 2013
 *      Author: acf
 */

#ifndef LOGSERVERSINK_H_
#define LOGSERVERSINK_H_

#include "LogSinkBase.hpp"
#include <list>

/*!
 *  A file sink for the logged events.
 */
class LogServerSink : public LogSinkBase {
public:
    LogServerSink();
    virtual ~LogServerSink();

    /*!
     *  Write an event to the logfile.
     *  \param event The event to log.
     */
    void logEvent( LogEvent event );

    int startServer( unsigned short port );

    int acceptor( bool blocking );

private:
    int tcpListen( unsigned short port );
    int acceptConnectionBlocking();
    int acceptConnectionNonBlocking();

    int m_listensd;
    std::list<int> m_connections;
};

#endif /* LOGFILESINK_H_ */

