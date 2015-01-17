#include "SocketConnection.hpp"

#include <unistd.h>

SocketConnection::SocketConnection( int nfd ) {
    fd = nfd;
    selectflags = Read | Error;

    writebufoffset = 0;
    writedone = true;

    readbufoffset = 0;
    readdone = true;
}

SocketConnection::~SocketConnection() {
    close( fd );
}
