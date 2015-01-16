#include "GraphHost.hpp"
#include <chrono>

#include <cstdio>
#include <cstring>
#include <strings.h>

#ifdef __VXWORKS__

#include <ioLib.h>
#include <pipeDrv.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include <sockLib.h>
#include <hostLib.h>
#include <selectLib.h>

#define be64toh(x) x

#else

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/select.h>
#include <stropts.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <endian.h>

#endif

#include <iterator>

GraphHost::GraphHost( int port ) :
        m_sendInterval( 5 ) {
    m_lastTime = 0;
    m_currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

    int pipefd[2];

    // Mark the thread as not running, this will be set to 1 by the thread
    m_running = false;

    // Store the port to listen on
    m_port = port;

    // Create a pipe for IPC with the thread
#ifdef __VXWORKS__
    pipeDevCreate( "/pipe/graphhost" , 10 , 100 );
    pipefd[0] = open( "/pipe/graphhost" , O_RDONLY , 0644 );
    pipefd[1] = open( "/pipe/graphhost" , O_WRONLY , 0644 );

    if ( pipefd[0] == -1 || pipefd[1] == -1 ) {
        return;
    }
#else
    int error = pipe( pipefd );
    if ( error == -1 ) {
        return;
    }
#endif

    m_ipcfd_r = pipefd[0];
    m_ipcfd_w = pipefd[1];

    /* Launch the thread */
    m_thread = new std::thread( [this] { sockets_threadmain(); } );
}

GraphHost::~GraphHost() {
    // Tell the other thread to stop
    write( m_ipcfd_w , "x" , 1 );

    // Join to the other thread
    m_thread->join();

    // Close file descriptors and clean up
    close( m_ipcfd_r );
    close( m_ipcfd_w );
}

int GraphHost::graphData( float value , std::string dataset ) {
    // This will only work if ints are the same size as floats
    static_assert( sizeof(float) == sizeof(uint32_t) ,
                   "float isn't 32 bits long" );

    m_currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

    struct graph_payload_t payload;
    decltype(m_currentTime) xtmp;
    uint32_t ytmp;

    if ( !m_running ) {
        return -1;
    }

    // Zero the payload structure
    std::memset( &payload , 0 , sizeof(struct graph_payload_t) );

    // Change to network byte order
    payload.type = 'd';

    // Swap bytes in x, and copy into the payload struct
    std::memcpy( &xtmp , &m_currentTime , sizeof(xtmp) );
    xtmp = be64toh( xtmp );
    std::memcpy( &payload.x , &xtmp , sizeof(xtmp) );

    // Swap bytes in y, and copy into the payload struct
    std::memcpy( &ytmp , &value , sizeof(ytmp) );
    ytmp = htonl( ytmp );
    std::memcpy( &payload.y , &ytmp , sizeof(ytmp) );

    std::strncpy( payload.dataset , dataset.c_str() , 15 );

    // Giant lock approach
    m_mutex.lock();

    // If the dataset name isn't in the list already, add it
    socket_addgraph( dataset );

    // Send the point to connected clients
    for ( SocketConnection& conn : m_connList ) {
        for ( std::string& dataset_str : conn.datasets ) {
            if ( dataset_str == dataset ) {
                // Send the value off
                sockets_queuewrite( conn , (char*)&payload ,
                        sizeof(struct graph_payload_t) );
            }
        }
    }

    m_mutex.unlock();

    return 0;
}

void GraphHost::setSendInterval( uint32_t milliseconds ) {
    m_sendInterval = milliseconds;
}

bool GraphHost::hasIntervalPassed() {
    m_currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

    return m_currentTime - m_lastTime > m_sendInterval;
}

void GraphHost::resetInterval() {
    m_lastTime = m_currentTime;
}

void GraphHost::sockets_threadmain() {
    int listenfd;
    int maxfd;
    uint8_t ipccmd;

    fd_set readfds;
    fd_set writefds;
    fd_set errorfds;

    // Listen on a socket
    listenfd = sockets_listen( m_port , AF_INET , 0 );
    if ( listenfd == -1 ) {
        return;
    }

    // Set the running flag after we've finished initializing everything
    m_running = true;

    while ( 1 ) {
        // Clear the fdsets
        FD_ZERO( &readfds );
        FD_ZERO( &writefds );
        FD_ZERO( &errorfds );

        // Reset the maxfd
        maxfd = listenfd;

        // Add the file descriptors to the list
        m_mutex.lock();
        for ( SocketConnection& conn : m_connList ) {
            if ( maxfd < conn.fd ) {
                maxfd = conn.fd;
            }
            if ( conn.selectflags & SocketConnection::Read ) {
                FD_SET( conn.fd , &readfds );
            }
            if ( conn.selectflags & SocketConnection::Write ) {
                FD_SET( conn.fd , &writefds );
            }
            if ( conn.selectflags & SocketConnection::Error ) {
                FD_SET( conn.fd , &errorfds );
            }
        }
        m_mutex.unlock();

        // Select on the listener fd
        FD_SET( listenfd , &readfds );

        // ipcfd will receive data when the thread needs to exit
        FD_SET( m_ipcfd_r , &readfds );

        // Select on the file descriptors
        select( maxfd+1 , &readfds , &writefds , &errorfds , nullptr );

        m_mutex.lock();
        auto conn = m_connList.begin();
        while ( conn != m_connList.end() ) {
            if ( FD_ISSET( conn->fd , &readfds ) ) {
                // Handle reading
                if ( sockets_readh( *conn ) == -1 ) {
                    conn = m_connList.erase( conn );
                    continue;
                }
            }
            if ( FD_ISSET( conn->fd , &writefds ) ) {
                // Handle writing
                sockets_writeh( *conn );
            }
            if ( FD_ISSET( conn->fd , &errorfds ) ) {
                // Handle errors
                conn = m_connList.erase( conn );
                continue;
            }

            conn++;
        }
        m_mutex.unlock();

        // Check for listener condition
        if ( FD_ISSET( listenfd , &readfds ) ) {
            // Accept connections
            sockets_accept( listenfd );
        }

        // Handle IPC commands
        if ( FD_ISSET( m_ipcfd_r , &readfds ) ) {
            read( m_ipcfd_r , (char*)&ipccmd , 1 );
            if ( ipccmd == 'x' ) {
                break;
            }
        }
    }

    // We're done, clear the running flag and clean up
    m_running = false;

    // Close the listener file descriptor
    m_mutex.lock();
    close( listenfd );
    m_mutex.unlock();
}

/* Listens on a specified port (listenport), and returns the file descriptor
 * to the listening socket.
 */
int GraphHost::sockets_listen( int port , sa_family_t sin_family ,
        uint32_t s_addr ) {
    struct sockaddr_in serv_addr;
    int error;
    int sd;

    // Create a TCP socket
    sd = socket( sin_family , SOCK_STREAM , 0 );
    if ( sd == -1 ) {
        perror( "" );
        return -1;
    }

    // Zero out the serv_addr struct
    std::memset( &serv_addr , 0 , sizeof(struct sockaddr_in) );

    // Set up the listener sockaddr_in struct
    serv_addr.sin_family = sin_family;
    serv_addr.sin_addr.s_addr = s_addr;
    serv_addr.sin_port = htons(port);

    // Bind the socket to the listener sockaddr_in
    error = bind( sd , reinterpret_cast<struct sockaddr*>( &serv_addr ) ,
            sizeof(struct sockaddr_in) );
    if ( error != 0 ) {
        perror( "" );
        close( sd );
        return -1;
    }

    // Listen on the socket for incoming connections
    error = listen( sd , 5 );
    if ( error != 0 ) {
        perror( "" );
        close( sd );
        return -1;
    }

    // Make sure we aren't killed by SIGPIPE
    signal( SIGPIPE , SIG_IGN );

    return sd;
}

void GraphHost::sockets_accept( int listenfd ) {
    int new_fd;
#ifdef __VXWORKS__
    int clilen;
#else
    unsigned int clilen;
#endif
    struct sockaddr_in cli_addr;
    int error;

    clilen = sizeof(cli_addr);

    // Accept a new connection
    new_fd = accept( listenfd , reinterpret_cast<struct sockaddr*>( &cli_addr ) ,
            &clilen );

    // Make sure that the file descriptor is valid
    if ( new_fd < 1 ) {
        perror( "" );
        return;
    }

#ifdef __VXWORKS__
    // Set the socket non-blocking
    int on = 1;
    error = ioctl( new_fd , (int)FIONBIO , on );
    if ( error == -1 ) {
        perror( "" );
        close( new_fd );
        return;
    }
#else
    // Set the socket non-blocking
    int flags = fcntl( new_fd , F_GETFL , 0 );
    if ( flags == -1 ) {
        perror( "" );
        close( new_fd );
        return;
    }

    error = fcntl( new_fd , F_SETFL , flags | O_NONBLOCK );
    if ( error == -1 ) {
        perror( "" );
        close( new_fd );
        return;
    }
#endif

    // Add it to the list, this makes it a bit non-thread-safe
    m_connList.emplace_front( new_fd );
}

int GraphHost::sockets_readh( SocketConnection& conn ) {
    int error;

    if ( conn.readdone ) {
        conn.readbufoffset = 0;
        conn.readbuf = std::string( 16 , 0 );
        conn.readdone = false;
    }

    error = recv( conn.fd , &conn.readbuf[0] , conn.readbuf.length() -
            conn.readbufoffset , 0 );
    if ( error < 1 ) {
        // recv(3) failed, so return failure so socket is closed
        return -1;
    }
    conn.readbufoffset += error;

    if ( conn.readbufoffset == conn.readbuf.length() ) {
        sockets_readdoneh( &conn.readbuf[0] , conn.readbuf.length() , conn );
        conn.readbufoffset = 0;
        conn.readdone = true;
    }

    return 0;
}

// Recieves 16 byte buffers which will be freed upon return
int GraphHost::sockets_readdoneh( char* inbuf , size_t bufsize ,
        SocketConnection& conn ) {
    inbuf[15] = 0;
    const char* graphstr = inbuf + 1;

    switch( inbuf[0] ) {
    case 'c':
        // Start sending data for the graph specified by graphstr
        conn.datasets.push_front( graphstr );
        break;
    case 'd':
        // Stop sending data for the graph specified by graphstr
        for ( auto i = m_graphList.begin() ; i != m_graphList.end() ; i++ ) {
            if ( *i == graphstr ) {
                conn.datasets.erase( i );
                break;
            }
        }
        break;
    case 'l':
        /* If this fails, we just ignore it. There's really nothing we can do
         * about it right now.
         */
        sockets_sendlist( conn );
    }

    return 0;
}

// Send to the client a list of available graphs
int GraphHost::sockets_sendlist( SocketConnection& conn ) {
    struct graph_list_t replydg;

    for ( auto i = m_graphList.begin() ; i != m_graphList.end() ; i++ ) {
        // Set up the response body, and queue it for sending
        std::memset( &replydg , 0 , sizeof(struct graph_list_t) );

        // Set the type of the datagram
        replydg.type = 'l';

        // Is this the last element in the list?
        if ( std::next(i) == m_graphList.end() ) {
            replydg.end = 1;
        }
        else {
            replydg.end = 0;
        }

        // Copy in the string
        std::strcpy( replydg.dataset , i->c_str() );

        // Queue the datagram for writing
        if ( sockets_queuewrite( conn , (char*)&replydg ,
                sizeof(struct graph_list_t)) == -1 ) {
            return -1;
        }
    }

    return 0;
}

// Write queued data to a socket when the socket becomes ready
int GraphHost::sockets_writeh( SocketConnection& conn ) {
    while ( 1 ) {
        // Get another buffer to send
        if ( conn.writedone ) {
            // There are no more buffers in the queue
            if ( conn.writequeue.empty() ) {
                // Stop selecting on write
                conn.selectflags &= ~SocketConnection::Write;

                return 0;
            }

            conn.writebuf = std::move( conn.writequeue.front() );
            conn.writebufoffset = 0;
            conn.writedone = false;
            conn.writequeue.pop();
        }

        // These descriptors are ready for writing
        conn.writebufoffset += send( conn.fd , &conn.writebuf[0] ,
                conn.writebuf.length() - conn.writebufoffset , 0 );

        // Have we finished writing the buffer?
        if ( conn.writebufoffset == conn.writebuf.length() ) {
            // Reset the write buffer
            conn.writebufoffset = 0;
            conn.writedone = true;
        }
        else {
            // We haven't finished writing, keep selecting
            return 0;
        }
    }

    // We always return from within the loop, this is unreachable
    return -1;
}

/* Queue a buffer for writing. Returns 0 on success, returns -1 if buffer
 * wasn't queued. Only one buffer can be queued for writing at a time.
 */
int GraphHost::sockets_queuewrite( SocketConnection& conn , char* buf ,
        size_t buflength ) {
    conn.writequeue.push( std::string( buf , buflength ) );

    // Select on write
    conn.selectflags |= SocketConnection::Write;
    write( m_ipcfd_w , "r" , 1 );

    return 0;
}

// If the dataset name isn't in the list already, add it
int GraphHost::socket_addgraph( std::string& dataset ) {
    // Add the graph name to the list of available graphs
    for ( std::string& elem : m_graphList ) {
        // Graph is already in list
        if ( elem == dataset ) {
            return 1;
        }
    }

    // Graph wasn't in the list, so add it
    m_graphList.push_front( dataset );

    return 0;
}
