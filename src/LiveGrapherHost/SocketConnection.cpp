// =============================================================================
// File Name: SocketConnection.cpp
// Description: Wrapper around graph client socket descriptors
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "SocketConnection.hpp"
#include "GraphHost.hpp"

#include <algorithm>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

std::vector<std::string> SocketConnection::graphNames;

SocketConnection::SocketConnection(int nfd, int ipcWriteSock) {
    fd = nfd;
    selectflags = Read | Error;
    m_ipcfd_w = ipcWriteSock;

    m_writebufoffset = 0;
    m_writedone = true;

    m_readbufoffset = 0;
    m_readdone = true;
}

SocketConnection::~SocketConnection() {
    close(fd);
}

int SocketConnection::readh() {
    int error;

    if (m_readdone) {
        m_readbufoffset = 0;
        m_readbuf = std::string(16, 0);
        m_readdone = false;
    }

    error = recv(fd, &m_readbuf[0], m_readbuf.length() -
                 m_readbufoffset, 0);
    if (error == 0 || (error == -1 && errno != EAGAIN)) {
        // recv(3) failed, so return failure so socket is closed
        return -1;
    }
    m_readbufoffset += error;

    if (m_readbufoffset == m_readbuf.length()) {
        // Add null terminator
        m_readbuf[15] = 0;
        readdoneh(m_readbuf);
        m_readbufoffset = 0;
        m_readdone = true;
    }

    return 0;
}

// Recieves 16 byte buffers
int SocketConnection::readdoneh(std::string& buf) {
    const char* graphstr = buf.c_str() + 1;

    switch (buf[0]) {
    case 'c':
        // Start sending data for the graph specified by graphstr
        if (std::find(datasets.begin(), datasets.end(),
                      graphstr) == datasets.end()) {
            datasets.push_back(graphstr);
        }
        break;
    case 'd':
        // Stop sending data for the graph specified by graphstr
        for (auto i = datasets.begin(); i != datasets.end(); i++) {
            if (*i == graphstr) {
                datasets.erase(i);
                break;
            }
        }
        break;
    case 'l':
        /* If this fails, we just ignore it. There's really nothing we can do
         * about it right now.
         */
        sendlist();
    }

    return 0;
}

// Send to the client a list of available graphs
int SocketConnection::sendlist() {
    struct graph_list_t replydg;

    // Set up the response body, and queue it for sending
    std::memset(&replydg, 0, sizeof(struct graph_list_t));

    // Set the type of the datagram
    replydg.type = 'l';

    for (unsigned int i = 0; i < graphNames.size(); i++) {
        // Is this the last element in the list?
        if (i + 1 == graphNames.size()) {
            replydg.end = 1;
        }
        else {
            replydg.end = 0;
        }

        // Copy in the string
        std::strcpy(replydg.dataset, graphNames[i].c_str());

        // Queue the datagram for writing
        if (queuewrite(replydg) == -1) {
            return -1;
        }
    }

    return 0;
}

// Write queued data to a socket when the socket becomes ready
int SocketConnection::writeh() {
    while (1) {
        // Get another buffer to send
        if (m_writedone) {
            // There are no more buffers in the queue
            if (m_writequeue.empty()) {
                // Stop selecting on write
                selectflags &= ~SocketConnection::Write;

                return 0;
            }

            m_writebuf = std::move(m_writequeue.front());
            m_writebufoffset = 0;
            m_writedone = false;
            m_writequeue.pop();
        }

        // These descriptors are ready for writing
        m_writebufoffset += send(fd, &m_writebuf[0],
                                 m_writebuf.length() - m_writebufoffset, 0);

        // Have we finished writing the buffer?
        if (m_writebufoffset == m_writebuf.length()) {
            // Reset the write buffer
            m_writebufoffset = 0;
            m_writedone = true;
        }
        else {
            // We haven't finished writing, keep selecting
            return 0;
        }
    }

    // We always return from within the loop, this is unreachable
    return -1;
}

