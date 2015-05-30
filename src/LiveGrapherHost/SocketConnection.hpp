// =============================================================================
// File Name: SocketConnection.hpp
// Description: Wrapper around graph client socket descriptors
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#ifndef SOCKET_CONNECTION_HPP
#define SOCKET_CONNECTION_HPP

#include <vector>
#include <string>
#include <queue>

#include <cstdint>

class SocketConnection {
public:
    enum selector {
        Read = 1,
        Write = 2,
        Error = 4
    };

    SocketConnection(int nfd, int ipcWriteSock);
    ~SocketConnection();
    SocketConnection(const SocketConnection&) = delete;
    SocketConnection& operator=(const SocketConnection&) = delete;

    int readPackets();
    void processPacket(std::string& buf);
    void sendList();
    void writePackets();
    template <class T>
    void queueWrite(T& buf);

    // Contains all graphs
    static std::vector<std::string> graphNames;

    int fd;
    uint8_t selectflags{Read | Error};
    std::vector<std::string> datasets;

private:
    int m_ipcfd_w;

    // Write buffer currently being written
    std::string m_writebuf; // The buffer that needs to be written into the socket
    size_t m_writebufoffset{0}; // How much has been written so far
    bool m_writedone{true};
    std::queue<std::string> m_writequeue;

    // Read buffer currently being read
    std::string m_readbuf;
    size_t m_readbufoffset{0};
    bool m_readdone{true};
};

#include "SocketConnection.inl"

#endif // SOCKET_CONNECTION_HPP

