// =============================================================================
// Description: The host for the LiveGrapher real-time graphing application
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "GraphHost.hpp"

#include <cstring>

#ifdef __VXWORKS__

#include <cstdio>
#include <pipeDrv.h>

#include <sockLib.h>
#include <hostLib.h>
#include <selectLib.h>

#define be64toh(x) x

#else

#include <netinet/in.h>
#include <signal.h>
#include <fcntl.h>
#include <endian.h>

#endif

GraphHost::GraphHost(int port) {
    m_currentTime = duration_cast<milliseconds>(
        system_clock::now().time_since_epoch()).count();

    // Store the port to listen on
    m_port = port;

    // Create a pipe for IPC with the thread
    int pipefd[2];
#ifdef __VXWORKS__
    pipeDevCreate("/pipe/graphhost", 10, 100);
    pipefd[0] = open("/pipe/graphhost", O_RDONLY, 0644);
    pipefd[1] = open("/pipe/graphhost", O_WRONLY, 0644);

    if (pipefd[0] == -1 || pipefd[1] == -1) {
        return;
    }
#else
    if (pipe(pipefd) == -1) {
        return;
    }
#endif

    m_ipcfd_r = pipefd[0];
    m_ipcfd_w = pipefd[1];

    // Launch the thread
    m_thread = std::thread([this] { socket_threadmain(); });
}

GraphHost::~GraphHost() {
    // Tell the other thread to stop
    write(m_ipcfd_w, "x", 1);

    // Join to the other thread
    m_thread.join();

    // Close file descriptors and clean up
    close(m_ipcfd_r);
    close(m_ipcfd_w);
}

int GraphHost::GraphData(float value, std::string dataset) {
    if (!m_running) {
        return -1;
    }

    // This will only work if ints are the same size as floats
    static_assert(sizeof(float) == sizeof(uint32_t),
                  "float isn't 32 bits long");

    m_currentTime = duration_cast<milliseconds>(
        system_clock::now().time_since_epoch()).count();

    struct graph_payload_t payload;
    decltype(m_currentTime)xtmp;
    uint32_t ytmp;

    // Zero the payload structure
    std::memset(&payload, 0, sizeof(struct graph_payload_t));

    // Change to network byte order
    payload.type = 'd';

    // Swap bytes in x, and copy into the payload struct
    std::memcpy(&xtmp, &m_currentTime, sizeof(xtmp));
    xtmp = be64toh(xtmp);
    std::memcpy(&payload.x, &xtmp, sizeof(xtmp));

    // Swap bytes in y, and copy into the payload struct
    std::memcpy(&ytmp, &value, sizeof(ytmp));
    ytmp = htonl(ytmp);
    std::memcpy(&payload.y, &ytmp, sizeof(ytmp));

    std::strncpy(payload.dataset, dataset.c_str(), 15);

    m_mutex.lock();

    // If the dataset name isn't in the list already, add it
    AddGraph(dataset);

    // Send the point to connected clients
    for (auto& conn : m_connList) {
        for (const auto& dataset_str : conn->datasets) {
            if (dataset_str == dataset) {
                // Send the value off
                conn->queueWrite(payload);
            }
        }
    }

    m_mutex.unlock();

    return 0;
}

bool GraphHost::HasIntervalPassed() {
    m_currentTime = duration_cast<milliseconds>(
        system_clock::now().time_since_epoch()).count();

    return m_currentTime - m_lastTime > m_sendInterval;
}

void GraphHost::ResetInterval() {
    m_lastTime = m_currentTime;
}

void GraphHost::socket_threadmain() {
    int listenfd;
    int maxfd;
    uint8_t ipccmd = 0;

    fd_set readfds;
    fd_set writefds;
    fd_set errorfds;

    // Listen on a socket
    listenfd = socket_listen(m_port, 0);
    if (listenfd == -1) {
        return;
    }

    // Set the running flag after we've finished initializing everything
    m_running = true;

    while (ipccmd != 'x') {
        // Clear the fdsets
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        FD_ZERO(&errorfds);

        // Reset the maxfd
        maxfd = listenfd;

        // Add the file descriptors to the list
        m_mutex.lock();
        for (auto& conn : m_connList) {
            if (maxfd < conn->fd) {
                maxfd = conn->fd;
            }
            if (conn->selectflags & SocketConnection::Read) {
                FD_SET(conn->fd, &readfds);
            }
            if (conn->selectflags & SocketConnection::Write) {
                FD_SET(conn->fd, &writefds);
            }
            if (conn->selectflags & SocketConnection::Error) {
                FD_SET(conn->fd, &errorfds);
            }
        }
        m_mutex.unlock();

        // Select on the listener fd
        FD_SET(listenfd, &readfds);

        // ipcfd will receive data when the thread needs to exit
        FD_SET(m_ipcfd_r, &readfds);

        // Select on the file descriptors
        select(maxfd + 1, &readfds, &writefds, &errorfds, nullptr);

        m_mutex.lock();
        auto conn = m_connList.begin();
        while (conn != m_connList.end()) {
            if (FD_ISSET((*conn)->fd, &readfds)) {
                // Handle reading
                if ((*conn)->readPackets() == -1) {
                    conn = m_connList.erase(conn);
                    continue;
                }
            }
            if (FD_ISSET((*conn)->fd, &writefds)) {
                // Handle writing
                (*conn)->writePackets();
            }
            if (FD_ISSET((*conn)->fd, &errorfds)) {
                // Handle errors
                conn = m_connList.erase(conn);
                continue;
            }

            conn++;
        }
        m_mutex.unlock();

        // Check for listener condition
        if (FD_ISSET(listenfd, &readfds)) {
            // Accept connections
            int fd = socket_accept(listenfd);

            if (fd != -1) {
                m_mutex.lock();
                // Add it to the list, this makes it a bit non-thread-safe
                m_connList.emplace_back(std::make_unique<SocketConnection>(fd,
                                                                           m_ipcfd_w));
                m_mutex.unlock();
            }
        }

        // Handle IPC commands
        if (FD_ISSET(m_ipcfd_r, &readfds)) {
            read(m_ipcfd_r, (char*) &ipccmd, 1);
        }
    }

    // We're done, clear the running flag and clean up
    m_running = false;

    // Close the listener file descriptor
    close(listenfd);
}

/* Listens on a specified port (listenport), and returns the file descriptor
 * to the listening socket.
 */
int GraphHost::socket_listen(int port, uint32_t s_addr) {
    struct sockaddr_in serv_addr;
    int sd = -1;

    try {
        // Create a TCP socket
        sd = socket(AF_INET, SOCK_STREAM, 0);
        if (sd == -1) {
            throw -1;
        }

        // Allow rebinding to the socket later if the connection is interrupted
#ifndef __VXWORKS__
        int optval = 1;
        setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
#endif

        // Zero out the serv_addr struct
        std::memset(&serv_addr, 0, sizeof(struct sockaddr_in));

        // Set up the listener sockaddr_in struct
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = s_addr;
        serv_addr.sin_port = htons(port);

        // Bind the socket to the listener sockaddr_in
        if (bind(sd, reinterpret_cast<struct sockaddr*>(&serv_addr),
                 sizeof(struct sockaddr_in)) != 0) {
            throw -1;
        }

        // Listen on the socket for incoming connections
        if (listen(sd, 5) != 0) {
            throw -1;
        }
    }
    catch (int e) {
        perror("");
        if (sd != -1) {
            close(sd);
        }
        return -1;
    }

    // Make sure we aren't killed by SIGPIPE
    signal(SIGPIPE, SIG_IGN);

    return sd;
}

int GraphHost::socket_accept(int listenfd) {
#ifdef __VXWORKS__
    int clilen;
#else
    unsigned int clilen;
#endif
    struct sockaddr_in cli_addr;

    clilen = sizeof(cli_addr);

    int new_fd = -1;

    try {
        // Accept a new connection
        new_fd = accept(listenfd,
                        reinterpret_cast<struct sockaddr*>(&cli_addr), &clilen);

        // Make sure that the file descriptor is valid
        if (new_fd == -1) {
            throw -1;
        }

#ifdef __VXWORKS__
        // Set the socket non-blocking
        int on = 1;
        if (ioctl(new_fd, (int) FIONBIO, on) == -1) {
            throw -1;
        }
#else
        // Set the socket non-blocking
        int flags = fcntl(new_fd, F_GETFL, 0);
        if (flags == -1) {
            throw -1;
        }

        if (fcntl(new_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
            throw -1;
        }
#endif
    }
    catch (int e) {
        perror("");
        if (new_fd != -1) {
            close(new_fd);
        }
        return -1;
    }

    return new_fd;
}

// If the dataset name isn't in the list already, add it
int GraphHost::AddGraph(const std::string& dataset) {
    // Add the graph name to the list of available graphs
    for (const auto& elem : SocketConnection::graphNames) {
        // Graph is already in list
        if (elem == dataset) {
            return 1;
        }
    }

    // Graph wasn't in the list, so add it
    SocketConnection::graphNames.push_back(dataset);

    return 0;
}

