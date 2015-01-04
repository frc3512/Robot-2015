#include "graphhost.hpp"

#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/select.h>
#include <stropts.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

#include <algorithm>
#include <iterator>

graphhost_t::graphhost_t( int port ) {
    int pipefd[2];
    int error;

    /* Mark the thread as not running, this will be set to 1 by the thread */
    m_running = false;

    /* Store the port to listen on */
    m_port = port;

    /* Create a pipe for IPC with the thread */
    error = pipe(pipefd);
    if(error == -1) {
        return;
    }

    m_ipcfd_r = pipefd[0];
    m_ipcfd_w = pipefd[1];

    /* Launch the thread */
    m_thread = new std::thread( [this] { sockets_threadmain(); } );
}

graphhost_t::~graphhost_t() {
    // Tell the other thread to stop
    write(m_ipcfd_w, "x", 1);

    m_thread->join();

    close( m_ipcfd_r );
    close( m_ipcfd_w );
}

// We assume that a float is 32 bits long
int graphhost_t::graphData( float x , float y , std::string dataset ) {
    static_assert( sizeof(float) == sizeof(uint32_t) , "float isn't 32 bits long" );

    struct graph_payload_t payload;
    uint32_t tmp;

    if ( !m_running ) {
        return -1;
    }

    // Zero the payload structure
    memset((void*)&payload, 0x00, sizeof(struct graph_payload_t));

    // Change to network byte order
    payload.type = 'd';

    /* Swap bytes in x, and copy into the payload struct */
    memcpy( &tmp , &x , sizeof(uint32_t) );
    tmp = htonl( tmp );
    memcpy( &payload.x , &tmp , sizeof(uint32_t) );

    // Swap bytes in y, and copy into the payload struct
    memcpy( &tmp , &y , sizeof(uint32_t) );
    tmp = htonl( tmp );
    memcpy( &payload.y , &tmp , sizeof(uint32_t) );

    strncpy( payload.dataset , dataset.c_str() , 15 );

    // Giant lock approach
    m_mutex.lock();

    // Search for the graph name in the list of available graphs
    for ( std::string& elem : m_graphList ) {
        // Graph is already in list
        if ( elem == dataset ) {
            return 1;
        }
    }

    // Graph wasn't in the list, so add it
    m_graphList.push_front( dataset );

    // Send the point to connected clients
    for ( struct socketconn_t& conn : m_connList ) {
      for ( std::string& dataset_str : conn.datasets ) {

          if( dataset_str == dataset ) {
              // Send the value off
              sockets_queuewrite(conn, (uint8_t*)&payload, sizeof(struct graph_payload_t));
          }
      }
    }

    m_mutex.unlock();

    return 0;
}

void graphhost_t::sockets_threadmain() {
    int listenfd;
    int maxfd;
    uint8_t ipccmd;

    fd_set readfds;
    fd_set writefds;
    fd_set errorfds;

    // Listen on a socket
    listenfd = sockets_listen_int(m_port, AF_INET, 0x00000000);
    if(listenfd == -1) {
        return;
    }

    // Set the running flag after we've finished initializing everything
    m_running = true;

    while ( 1 ) {
        // Clear the fdsets
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        FD_ZERO(&errorfds);

        // Reset the maxfd
        maxfd = listenfd;

        // Add the file descriptors to the list
        m_mutex.lock();
        for ( struct socketconn_t& conn : m_connList ) {
            if(conn.orphan == 1) continue;

            if(maxfd < conn.fd) {
                maxfd = conn.fd;
            }
            if(conn.selectflags & SOCKET_READ) {
                FD_SET(conn.fd, &readfds);
            }
            if(conn.selectflags & SOCKET_WRITE) {
                FD_SET(conn.fd, &writefds);
            }
            if(conn.selectflags & SOCKET_ERROR) {
                FD_SET(conn.fd, &errorfds);
            }
        }
        m_mutex.unlock();

        // Select on the listener fd
        FD_SET(listenfd, &readfds);

        // ipcfd will recieve data when the thread needs to exit
        FD_SET(m_ipcfd_r, &readfds);

        // Select on the file descrpitors
        select(maxfd+1, &readfds, &writefds, &errorfds, nullptr);

        m_mutex.lock();
        for ( struct socketconn_t& conn : m_connList ) {
            if(conn.orphan == 1) continue;

            if(FD_ISSET(conn.fd, &readfds)) {
                // Handle reading
                sockets_readh( conn );
            }
            if(FD_ISSET(conn.fd, &writefds)) {
                // Handle writing
                sockets_writeh( conn );
            }
            if(FD_ISSET(conn.fd, &errorfds)) {
                // Handle errors
                conn.orphan = 1;
            }
        }

        // Close all the file descriptors marked for closing
        sockets_clear_orphans(m_connList);
        m_mutex.unlock();

        // Check for listener condition
        if(FD_ISSET(listenfd, &readfds)) {
            // Accept connections
            sockets_accept(m_connList, listenfd);
        }

        /* Handle IPC commands */
        if(FD_ISSET(m_ipcfd_r, &readfds)) {
            read(m_ipcfd_r,(char*)&ipccmd, 1);
            if(ipccmd == 'x') {
                break;
            }
        }
    }

    // We're done, clear the running flag and clean up
    m_running = false;
    m_mutex.lock();

    // Mark all the open file descriptors for closing
    for ( struct socketconn_t& elem : m_connList ) {
        elem.orphan = 1;
        /* We don't need to delete the element from the
           because we just delete all of them below. */
    }

    // Actually close all the open file descriptors
    sockets_clear_orphans( m_connList );

    // Free the list of connections
    m_connList.clear();

    /* Close the listener file descriptor */
    close(listenfd);

    /* Clean up & free the global dataset list */
    m_graphList.clear();

    /* Destroy the mutex */
    m_mutex.unlock();
}

/* Listens on a specified port (listenport), and returns the file
 * descriptor to the listening socket.
 */
int sockets_listen_int(int port, sa_family_t sin_family, uint32_t s_addr) {
    struct sockaddr_in serv_addr;
    int error;
    int sd;

    /* Create a TCP socket */
    sd = socket(sin_family, SOCK_STREAM, 0);
    if(sd == -1){
        perror("");
        return -1;
    }

    /* Zero out the serv_addr struct */
    bzero((char *) &serv_addr, sizeof(struct sockaddr_in));

    /* Set up the listener sockaddr_in struct */
    serv_addr.sin_family = sin_family;
    serv_addr.sin_addr.s_addr = s_addr;
    serv_addr.sin_port = htons(port);

    /* Bind the socket to the listener sockaddr_in */
    error = bind(sd, (struct sockaddr *) &serv_addr,
    sizeof(struct sockaddr_in));
    if(error != 0){
        perror("");
        close(sd);
        return -1;
    }

    /* Listen on the socket for incoming conncetions */
    error = listen(sd, 5);
    if(error != 0){
        perror("");
        close(sd);
        return -1;
    }

    /* Make sure we aren't killed by SIGPIPE */
    signal(SIGPIPE, SIG_IGN);

    return sd;
}

void sockets_accept(std::list<struct socketconn_t>& connlist, int listenfd) {
    int new_fd;
    unsigned int clilen;
    struct sockaddr_in cli_addr;
    int error;
    int flags;

    clilen = sizeof(struct sockaddr_in);

    // Accept a new connection
    new_fd = accept(listenfd, (struct sockaddr*) &cli_addr,
            &clilen);

    // Make sure that the file descriptor is valid
    if ( new_fd < 1 ) {
        perror("");
        return;
    }

    /* Set the socket non-blocking. */
    flags = fcntl(new_fd, F_GETFL, 0);
    if(flags == -1) {
        perror("");
        close(new_fd);
        return;
    }

    error = fcntl(new_fd, F_SETFL, flags | O_NONBLOCK);
    if(error == -1) {
        perror("");
        close(new_fd);
        return;
    }

    struct socketconn_t conn;
    conn.fd = new_fd;
    conn.selectflags = SOCKET_READ | SOCKET_ERROR;

    conn.writebuf = nullptr;
    conn.writebuflength = 0;
    conn.writebufoffset = 0;

    conn.readbuf = nullptr;
    conn.readbuflength = 0;
    conn.readbufoffset = 0;

    conn.orphan = 0;

    // Add it to the list, this makes it a bit non-thread-safe
    connlist.push_front( std::move(conn) );
}

/* NOTE: Does not remove the element from the list */
void sockets_remove_orphan(struct socketconn_t& conn) {
    // Give up on the current write buffer
    if ( conn.writebuf != nullptr ) {
        delete[] conn.writebuf;
    }

    // Give up on the current read buffer
    if ( conn.readbuf != nullptr ) {
        delete[] conn.readbuf;
    }

    // Give up on all other queued buffers too
    struct writebuf_t writebuf;
    while ( !conn.writequeue.empty() ) {
        writebuf = conn.writequeue.front();
        delete[] writebuf.buf;
        conn.writequeue.pop();
    }

    // Free it when we get back to it, this is a hack
    conn.orphan = 1;

    close(conn.fd);
}

// Closes and clears orphans from the list
void sockets_clear_orphans( std::list<struct socketconn_t>& list ) {
    auto i = list.begin();
    while ( i != list.end() ) {
        if ( (*i).orphan == 1 ) {
            sockets_remove_orphan( *i );
            i = list.erase( i );
        }
        else {
            i++;
        }
    }
}

int graphhost_t::sockets_readh( struct socketconn_t& conn ) {
    int error;

    if(conn.readbuflength == 0) {
        conn.readbufoffset = 0;
        conn.readbuflength = 16; /* This should be configurable somewhere */
        conn.readbuf = new uint8_t[conn.readbuflength];
    }

    error = recv(conn.fd, (char*)conn.readbuf, conn.readbuflength - conn.readbufoffset, 0);
    if(error < 1) {
        /* Clean up the socket here */
        conn.orphan = 1;
        return 0;
    }
    conn.readbufoffset += error;

    if(conn.readbufoffset == conn.readbuflength) {
        sockets_readdoneh(conn.readbuf, conn.readbuflength, conn);
        conn.readbufoffset = 0;
        conn.readbuflength = 0;
        delete[] conn.readbuf;
        conn.readbuf = nullptr;
    }

    return 0;
}

/* Recieves 16 byte buffers which will be freed upon return */
int graphhost_t::sockets_readdoneh(uint8_t *inbuf, size_t bufsize, struct socketconn_t& conn) {
    inbuf[15] = 0;
    const char* graphstr = ((char *)inbuf)+1;

    switch(inbuf[0]) {
    case 'c':
        /* Start sending data for the graph specified by graphstr. */
        conn.datasets.push_front( graphstr );
        break;
    case 'd':
        /* Stop sending data for the graph specified by graphstr. */

        for ( auto i = m_graphList.begin() ; i != m_graphList.end() ; i++ ) {
            if ( *i == graphstr ) {
                conn.datasets.erase( i );
                break;
            }
        }
        break;
    case 'l':
        /* If this fails, we just ignore it. There's really nothing we can
        do about it right now. */
        sockets_sendlist( conn );
    }

    return 0;
}

/* Send to the client a list of available graphs */
int graphhost_t::sockets_sendlist(struct socketconn_t& conn) {
    struct graph_list_t replydg;

    for ( auto i = m_graphList.begin() ; i != m_graphList.end() ; i++ ) {
        /* Set up the response body, and queue it for sending. */
        memset((void *)&replydg, 0x00, sizeof(struct graph_list_t));

        /* Set the type of the datagram. */
        replydg.type = 'l';

        /* Is this the last element in the list? */
        if ( std::next(i) == m_graphList.end() ) {
          replydg.end = 1;
        }
        else {
          replydg.end = 0;
        }

        /* Copy in the string */
        strcpy(replydg.dataset, i->c_str());

        /* Queue the datagram for writing */
        if(sockets_queuewrite(conn, (uint8_t*)&replydg, sizeof(struct graph_list_t)) == -1) {
            return -1;
        }
    }

    return 0;
}

/* Write queued data to a socket when the socket becomes ready */
int graphhost_t::sockets_writeh(struct socketconn_t& conn) {
    struct writebuf_t writebuf;

    while ( 1 ) {
        /* Get another buffer to send */
        if(conn.writebuflength == 0) {
            /* There are no more buffers in the queue */
            if ( conn.writequeue.empty() ) {
                /* Stop selecting on write */
                conn.selectflags &= ~(SOCKET_WRITE);

                return 0;
            }

            writebuf = conn.writequeue.front();
            conn.writequeue.pop();

            conn.writebuf = writebuf.buf;
            conn.writebuflength = writebuf.buflength;
            conn.writebufoffset = 0;
        }

        /* These descriptors are ready for writing */
        conn.writebufoffset += send(conn.fd, (char*)conn.writebuf, conn.writebuflength - conn.writebufoffset, 0);

        /* Have we finished writing the buffer? */
        if(conn.writebufoffset == conn.writebuflength) {
            /* Reset the write buffer */
            conn.writebuflength = 0;
            conn.writebufoffset = 0;
            delete[] conn.writebuf;
            conn.writebuf = nullptr;
        }
        else {
            /* We haven't finished writing, keep selecting. */
            return 0;
        }
    }

    /* We always return from within the loop, this is unreachable */
    return -1;
}

/* Queue a buffer for writing. Returns 0 on success, returns -1 if buffer
 * wasn't queued. Only one buffer can be queued for writing at a time.
 */
int graphhost_t::sockets_queuewrite( struct socketconn_t& conn , uint8_t *buf , size_t buflength ) {
    struct writebuf_t writebuf;

    writebuf.buf = new uint8_t[buflength];
    writebuf.buflength = buflength;
    memcpy(writebuf.buf, buf, buflength);

    conn.writequeue.push( writebuf );

    /* Select on write */
    conn.selectflags |= SOCKET_WRITE;
    write(m_ipcfd_w, "r", 1);

    return 0;
}
