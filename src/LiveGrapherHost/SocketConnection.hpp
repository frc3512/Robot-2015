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

    SocketConnection(int nfd);
    virtual ~SocketConnection();
    SocketConnection(const SocketConnection& rhs) = delete;

    std::vector<std::string> datasets;
    int fd;
    uint8_t selectflags;

    // Write buffer currently being written
    std::string writebuf; // The buffer that needs to be written into the socket
    size_t writebufoffset; // How much has been written so far
    bool writedone;
    std::queue<std::string> writequeue;

    // Read buffer currently being read
    std::string readbuf;
    size_t readbufoffset;
    bool readdone;
};

#endif // SOCKET_CONNECTION_HPP

