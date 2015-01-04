/*
 * LogServerSink.cpp
 *
 *  Created on: Nov 10, 2013
 *      Author: acf
 */

#include "LogServerSink.h"
#include <stdio.h>
#include <stdint.h>
//#include <netinet/sctp.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

LogServerSink::LogServerSink()
{
    m_listensd = -1;
}

LogServerSink::~LogServerSink()
{
    // TODO Auto-generated destructor stub
    if(m_listensd >= 0) {
        close(m_listensd);
    }
}

void LogServerSink::logEvent(LogEvent event)
{
    std::list<int>::iterator it;
    std::list<int>::iterator cur;
    int ok;

    for(it = m_connections.begin(); it != m_connections.end(); ) {
        cur = it;
        it++;

        ok = send(*cur, event.toFormattedString().c_str(),
                event.toFormattedString().length(), 0);
        if(ok != (ssize_t) event.toFormattedString().length()) {
            close(*cur);
            m_connections.erase(cur);
        }
    }
}

int LogServerSink::startServer(unsigned short port) {
    m_listensd = tcpListen(port);
    if(m_listensd < 0) return -1;

    /* Ignore SIGPIPE */
    signal(SIGPIPE, SIG_IGN);

    return 0;
}

int LogServerSink::acceptConnectionBlocking() {
    socklen_t addrlen;
    struct sockaddr_in addr;
    int sd;

    addrlen = sizeof(struct sockaddr_in);
    sd = accept(m_listensd, (struct sockaddr *) &addr, (socklen_t *) &addrlen);
    if(sd < 0) return -1;
    m_connections.push_back(sd);

    return 0;
}

int LogServerSink::acceptConnectionNonBlocking() {
    int ok;
    struct timeval timeout;
    fd_set readfds;
    int accepted;

    accepted = 1;
    while(accepted) {
        FD_ZERO(&readfds);
        FD_SET(m_listensd, &readfds);
        timeout.tv_sec = 0;
        timeout.tv_usec = 0;
        ok = select(m_listensd+1, &readfds, nullptr, nullptr, &timeout);
        if(ok < 0) return -1;
        accepted = 0;
        if(FD_ISSET(m_listensd, &readfds)) {
            if(acceptConnectionBlocking() == -1) return -1;
            accepted = 1;
        }
    }

    return 0;
}

int LogServerSink::acceptor(bool blocking) {
    if(blocking) {
        return acceptConnectionBlocking();
    }else{
        return acceptConnectionNonBlocking();
    }
}

/* Listens on a specified port and returns the listener file descriptor,
   or -1 on error. */
int LogServerSink::tcpListen(unsigned short port) {
  int sd;
  int error;
  struct sockaddr_in addr;

  /* Create a socket */
  sd = socket(AF_INET, SOCK_STREAM, 0);
  if(sd < 1) return -1;

  /* Bind the socket to the address and port we
     want to listen on. */
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;
  error = bind(sd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
  if(error != 0) {
    close(sd);
    return -1;
  }

  /* Start the socket listening with a backlog
     of ten connections. */
  error = listen(sd, 10);
  if(error != 0) {
    close(sd);
    return -1;
  }

  return sd;
}
