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

#include <string>

#include <sys/times.h>

struct graphhost_t;

class GraphHost {
public:
    GraphHost(int port);
    virtual ~GraphHost();

    /* * Send data (y value) for a given dataset to remote client
     * * The current time is sent as the x value
     * * Return values:
     *       -1 = host not running
     *       0 = data sent successfully
     */
    int graphData(float value, std::string dataset);

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

    // Reset startTime to current system time (relative time is reset to zero)
    void resetTime();

private:
    struct graphhost_t *m_inst;

    // Last time data was graphed
    uint32_t m_lastTime;

    // Starting time used as offset
    uint32_t m_startTime;

    // Time interval after which data is sent to graph (in milliseconds per sample)
    uint32_t m_sendInterval;

    // Used as a temp variables in graphData(2)
    uint32_t m_currentTime;
    struct timeval m_rawTime;
};

#endif // GRAPHHOST_HPP
