#ifndef _GRAPHHOST_H
#define _GRAPHHOST_H

#include <cstdint>

#include <list>
#include <queue>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

class graphhost_t {
public:
    graphhost_t( int port );
    virtual ~graphhost_t();

    int graphData(float x, float y, std::string dataset);

private:
    std::thread* m_thread;
    std::mutex m_mutex;
    std::atomic<bool> m_running;
    int m_ipcfd_r;
    int m_ipcfd_w;
    int m_port;
    std::list<std::string> m_graphList;
    std::list<struct socketconn_t> m_connList;

    void sockets_threadmain();

    int sockets_readh( struct socketconn_t& conn );
    int sockets_readdoneh(uint8_t *inbuf, size_t bufsize, struct socketconn_t& conn);
    int sockets_sendlist(struct socketconn_t& conn);
    int sockets_writeh(struct socketconn_t& conn);
    int sockets_queuewrite(struct socketconn_t& conn, uint8_t *buf, size_t buflength);
};

struct socketconn_t {
  int fd;

  /* char *dataset; */
  std::list<std::string> datasets;
  uint8_t selectflags;

  int orphan;

  /* Write buffer currently being written */
  uint8_t *writebuf; /* The buffer that needs to be written into the socket */
  size_t writebuflength; /* The length of the buffer to be written */
  size_t writebufoffset; /* How much has been written so far */
  std::queue<struct writebuf_t> writequeue;

  /* Read buffer currently being read */
  uint8_t *readbuf;
  size_t readbuflength;
  size_t readbufoffset;
};

struct writebuf_t {
  // Write buffer
  uint8_t *buf; // The buffer that needs to be written into the socket
  uint32_t buflength; // The length of the buffer to be written
};

struct [[gnu::packed]] graph_payload_t {
    char type; // Set to 'd' to identify this as a graph payload packet
    char dataset[15];
    float x;
    float y;
};

struct [[gnu::packed]] graph_list_t {
    char type;
    char dataset[15];
    char end;
    char pad[7];
};

#define SOCKET_READ 1
#define SOCKET_WRITE 2
#define SOCKET_ERROR 4

// Internal functions

int sockets_listen_int(int port, sa_family_t sin_family, uint32_t s_addr);

void sockets_accept(std::list<struct socketconn_t>& connlist, int listenfd);

void sockets_remove_orphan(struct socketconn_t& conn);

void sockets_clear_orphans(std::list<struct socketconn_t>& list);

#endif /* _GRAPHHOST_H */
