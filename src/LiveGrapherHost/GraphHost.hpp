// Copyright (c) FRC Team 3512, Spartatroniks 2015-2016. All Rights Reserved.

#pragma once

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
 *     GraphHost pidGraph(3513);
 *     pidGraph.setSendInterval(5ms);
 *
 *     if (pidGraph.hasIntervalPassed()) {
 *         pidGraph.graphData(frisbeeShooter.getRPM(), "PID0");
 *         pidGraph.graphData(frisbeeShooter.getTargetRPM(), "PID1");
 *
 *         pidGraph.resetInterval();
 *     }
 */

#include <stdint.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

using namespace std::chrono;
using namespace std::chrono_literals;

#include "SocketConnection.hpp"

struct[[gnu::packed]] graph_payload_t {
    char type;  // Set to 'd' to identify this as a graph payload packet
    char dataset[15];
    uint64_t x;
    float y;
};

struct[[gnu::packed]] graph_list_t {
    char type;
    char dataset[15];
    char end;
    char pad[11];
};

/**
 * The host for the LiveGrapher real-time graphing application
 */
class GraphHost {
public:
    explicit GraphHost(int port);
    ~GraphHost();

    /* * Send data (y value) for a given dataset to remote client
     * * The current time is sent as the x value
     * * Return values:
     *       -1 = host not running
     *       0 = data sent successfully
     */
    int GraphData(float value, std::string dataset);

    /* Sets time interval after which data is sent to graph (milliseconds per
     * sample)
     */
    template <typename Rep, typename Period>
    void SetSendInterval(const std::chrono::duration<Rep, Period>& time);

    /* Returns true if the time between the last data transmission is greater
     * than the sending interval time
     */
    bool HasIntervalPassed();

    /* Resets time interval passed since last data transmission (makes
     * hasIntervalPassed() return false)
     */
    void ResetInterval();

private:
    // Last time data was graphed
    uint64_t m_lastTime = 0;

    /* Time interval after which data is sent to graph (in milliseconds per
     * sample)
     */
    uint32_t m_sendInterval = 5;

    // Used as a temp variables in graphData(2)
    uint64_t m_currentTime;

    // Mark the thread as not running, this will be set to true by the thread
    std::atomic<bool> m_running{false};
    std::thread m_thread;
    std::mutex m_mutex;
    int m_ipcfd_r;
    int m_ipcfd_w;
    int m_port;
    std::vector<std::string> m_graphList;
    std::vector<std::unique_ptr<SocketConnection>> m_connList;

    void socket_threadmain();

    static int socket_listen(int port, uint32_t s_addr);
    static int socket_accept(int listenfd);
    int AddGraph(const std::string& dataset);
};

#include "GraphHost.inl"
