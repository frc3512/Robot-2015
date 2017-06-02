// Copyright (c) 2015-2017 FRC Team 3512. All Rights Reserved.

#include "SocketConnection.hpp"

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <utility>

#include "GraphHost.hpp"

#ifdef __VXWORKS__
#include <sockLib.h>
#endif

std::vector<std::string> SocketConnection::graphNames;

SocketConnection::SocketConnection(int nfd, int ipcWriteSock) {
    fd = nfd;
    m_ipcfd_w = ipcWriteSock;
}

SocketConnection::~SocketConnection() { close(fd); }

// Receives 16 byte buffers
int SocketConnection::readPackets() {
    if (m_readdone) {
        m_readbufoffset = 0;
        m_readbuf = std::string(16, 0);
        m_readdone = false;
    }

    int error =
        recv(fd, &m_readbuf[0], m_readbuf.length() - m_readbufoffset, 0);
    if (error == 0 || (error == -1 && errno != EAGAIN)) {
        // recv(3) failed, so return failure so socket is closed
        return -1;
    }
    m_readbufoffset += error;

    if (m_readbufoffset == m_readbuf.length()) {
        // Add null terminator
        m_readbuf[15] = 0;
        processPacket(m_readbuf);
        m_readbufoffset = 0;
        m_readdone = true;
    }

    return 0;
}

void SocketConnection::processPacket(std::string& buf) {
    const char* graphName = buf.c_str() + 1;

    switch (buf[0]) {
        case 'c':
            // Start sending data for the graph specified by graphName
            if (std::find(datasets.begin(), datasets.end(), graphName) ==
                datasets.end()) {
                datasets.push_back(graphName);
            }
            break;
        case 'd':
            // Stop sending data for the graph specified by graphName
            std::remove_if(datasets.begin(), datasets.end(),
                           [&](const auto& set) { return set == graphName; });
            break;
        case 'l':
            sendList();
    }
}

// Send to the client a list of available graphs
void SocketConnection::sendList() {
    struct graph_list_t replydg;

    // Set up the response body, and queue it for sending
    std::memset(&replydg, 0, sizeof(struct graph_list_t));

    // Set the type of the datagram
    replydg.type = 'l';

    for (unsigned int i = 0; i < graphNames.size(); i++) {
        // Is this the last element in the list?
        if (i + 1 == graphNames.size()) {
            replydg.end = 1;
        } else {
            replydg.end = 0;
        }

        // Copy in the string
        std::strncpy(replydg.dataset, graphNames[i].c_str(),
                     graphNames[i].size());

        // Queue the datagram for writing
        queueWrite(replydg);
    }
}

// Write queued data to a socket when the socket becomes ready
void SocketConnection::writePackets() {
    /* While the current buffer isn't done sending or there are more buffers to
     * send
     */
    while (!m_writedone || !m_writequeue.empty()) {
        // Get another buffer to send
        if (m_writedone) {
            m_writebuf = std::move(m_writequeue.front());
            m_writebufoffset = 0;
            m_writedone = false;
            m_writequeue.pop();
        }

        // These descriptors are ready for writing
        m_writebufoffset +=
            send(fd, &m_writebuf[0], m_writebuf.length() - m_writebufoffset, 0);

        // Have we finished writing the buffer?
        if (m_writebufoffset == m_writebuf.length()) {
            // Reset the write buffer
            m_writebufoffset = 0;
            m_writedone = true;
        } else {
            // We haven't finished writing, keep selecting
            return;
        }
    }

    // Stop selecting on write
    selectflags &= ~SocketConnection::Write;
}
