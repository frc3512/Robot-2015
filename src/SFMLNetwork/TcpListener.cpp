////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2012 Laurent Gomila (laurent.gom@gmail.com)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

#include "../SFML/Network/TcpListener.hpp"
#include "../SFML/Network/TcpSocket.hpp"
#include "Socket.hpp"
#include <iostream>

namespace sf {
TcpListener::TcpListener() :
    Socket(Tcp) {
}

unsigned short TcpListener::getLocalPort() const {
    if (getHandle() != -1) {
        // Retrieve informations about the local end of the socket
        sockaddr_in address;
        Socket::AddrLength size = sizeof(address);
        socklen_t temp = size;
        if (getsockname(getHandle(),
                        reinterpret_cast<sockaddr*>(&address),
                        &temp) != -1) {
            return ntohs(address.sin_port);
        }
    }

    // We failed to retrieve the port
    return 0;
}

Socket::Status TcpListener::listen(unsigned short port) {
    // Create the internal socket if it doesn't exist
    create();

    // Bind the socket to the specified port
    sockaddr_in address = Socket::createAddress(INADDR_ANY, port);
    if (bind(getHandle(), reinterpret_cast<sockaddr*>(&address),
             sizeof(address)) == -1) {
        // Not likely to happen, but...
        std::cerr << "Failed to bind listener socket to port " << port << "\n";
        return Error;
    }

    // Listen to the bound port
    if (::listen(getHandle(), 0) == -1) {
        // Oops, socket is deaf
        std::cerr << "Failed to listen to port " << port << "\n";
        return Error;
    }

    return Done;
}

void TcpListener::close() {
    // Simply close the socket
    Socket::close();
}

Socket::Status TcpListener::accept(TcpSocket& socket) {
    // Make sure that we're listening
    if (getHandle() == -1) {
        std::cerr <<
            "Failed to accept a new connection, the socket is not listening\n";
        return Error;
    }

    // Accept a new connection
    sockaddr_in address;
    Socket::AddrLength length = sizeof(address);
    socklen_t temp = length;
    int remote = ::accept(
        getHandle(), reinterpret_cast<sockaddr*>(&address), &temp);

    // Check for errors
    if (remote == -1) {
        return Socket::getErrorStatus();
    }

    // Initialize the new connected socket
    socket.close();
    socket.create(remote);

    return Done;
}
} // namespace sf

