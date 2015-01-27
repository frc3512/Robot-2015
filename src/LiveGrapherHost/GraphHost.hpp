#ifndef GRAPHHOST_HPP
#define GRAPHHOST_HPP

/* Usage:
 *
 * The GraphHost interface is started upon object initialization.
 *
 * Call graphData() to send data over the network to a LiveGrapher client.
 *
 * The time value in each data pair is handled internally.
 *
 * Use the function hasIntervalPassed() to limit the frequency of data sending
 * in looping situations.
 *
 * Example:
 *     GraphHost pidGraph( 3513 );
 *
 *     if ( pidGraph.hasIntervalPassed() ) {
 *         pidGraph.graphData( frisbeeShooter.getRPM() , "PID0" );
 *         pidGraph.graphData( frisbeeShooter.getTargetRPM() , "PID1" );
 *
 *         pidGraph.resetInterval();
 *     }
 */

#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <memory>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdint>

#include "SocketConnection.hpp"

struct [[gnu::packed]] graph_payload_t {
    char type; // Set to 'd' to identify this as a graph payload packet
    char dataset[15];
    uint64_t x;
    float y;
};

struct [[gnu::packed]] graph_list_t {
    char type;
    char dataset[15];
    char end;
    char pad[11];
};

class GraphHost {
public:
    GraphHost( int port );
    virtual ~GraphHost();

    /* * Send data (y value) for a given dataset to remote client
     * * The current time is sent as the x value
     * * Return values:
     *       -1 = host not running
     *       0 = data sent successfully
     */
    int graphData( float value , std::string dataset );

    /* Sets time interval after which data is sent to graph (milliseconds per
     * sample)
     */
    void setSendInterval( uint32_t milliseconds );

    /* Returns true if the time between the last data transmission is greater
     * than the sending interval time
     */
    bool hasIntervalPassed();

    /* Resets time interval passed since last data transmission (makes
     * hasIntervalPassed() return false)
     */
    void resetInterval();

private:
    // Last time data was graphed
    uint64_t m_lastTime;

    // Time interval after which data is sent to graph (in milliseconds per sample)
    uint32_t m_sendInterval;

    // Used as a temp variables in graphData(2)
    uint64_t m_currentTime;

    std::thread* m_thread;
    std::mutex m_mutex;
    std::atomic<bool> m_running;
    int m_ipcfd_r;
    int m_ipcfd_w;
    int m_port;
    std::vector<std::string> m_graphList;
    std::vector<std::unique_ptr<SocketConnection>> m_connList;

    void sockets_threadmain();

    static int sockets_listen( int port , sa_family_t sin_family ,
            uint32_t s_addr );
    void sockets_accept( int listenfd );
    int sockets_readh( std::unique_ptr<SocketConnection>& conn );
    int sockets_readdoneh( char* inbuf , size_t bufsize ,
            std::unique_ptr<SocketConnection>& conn );
    int sockets_sendlist( std::unique_ptr<SocketConnection>& conn );
    int sockets_writeh( std::unique_ptr<SocketConnection>& conn );
    int sockets_queuewrite( std::unique_ptr<SocketConnection>& conn , char* buf ,
            size_t buflength );
    int socket_addgraph( std::string& dataset );
};

#endif // GRAPHHOST_HPP
