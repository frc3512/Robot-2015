#include "GraphHost.hpp"
#include <chrono>

#include "graphhost_impl/graphhost.h"

GraphHost::GraphHost(int port) :
        m_inst( GraphHost_create(port) ) ,
        m_sendInterval( 5 ) {
    resetTime();
}

GraphHost::~GraphHost() {
    GraphHost_destroy(m_inst);
}

int GraphHost::graphData(float value, std::string dataset) {
    m_currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    return GraphHost_graphData(m_currentTime - m_startTime, value, dataset.c_str(), m_inst);
}

void GraphHost::setSendInterval( uint32_t milliseconds ) {
    m_sendInterval = milliseconds;
}

bool GraphHost::hasIntervalPassed() {
    m_currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    return m_currentTime - m_lastTime > m_sendInterval;
}

void GraphHost::resetInterval() {
    m_lastTime = m_currentTime;
}

void GraphHost::resetTime() {
    /* Store the current time into startTime as the fixed starting point
     * for our graph.
     */
    m_startTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    m_lastTime = m_startTime;
}
