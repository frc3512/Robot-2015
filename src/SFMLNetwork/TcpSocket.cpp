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

#include "../SFML/Network/TcpSocket.hpp"
#include "../SFML/Network/IpAddress.hpp"
#include "../SFML/Network/Packet.hpp"
#include "Socket.hpp"
#include <algorithm>
#include <cstring>
#include <iostream>

namespace sf {

TcpSocket::TcpSocket() :
Socket(Tcp) {

}

unsigned short TcpSocket::getLocalPort() const {
    if ( getHandle() != -1 ) {
        // Retrieve informations about the local end of the socket
        sockaddr_in address;
        Socket::AddrLength size = sizeof(address);
        socklen_t temp = size;
        if (getsockname(getHandle(), reinterpret_cast<struct sockaddr*>(&address), &temp) != -1) {
            return ntohs(address.sin_port);
        }
    }

    // We failed to retrieve the port
    return 0;
}

IpAddress TcpSocket::getRemoteAddress() const {
    if ( getHandle() != -1 ) {
        // Retrieve informations about the remote end of the socket
        sockaddr_in address;
        Socket::AddrLength size = sizeof(address);
        socklen_t temp = size;
        if (getpeername(getHandle(), reinterpret_cast<sockaddr*>(&address), &temp) != -1) {
            return IpAddress(ntohl(address.sin_addr.s_addr));
        }
    }

    // We failed to retrieve the address
    return IpAddress::None;
}

unsigned short TcpSocket::getRemotePort() const {
    if ( getHandle() != -1 ) {
        // Retrieve informations about the remote end of the socket
        sockaddr_in address;
        Socket::AddrLength size = sizeof(address);
        socklen_t temp = size;
        if (getpeername(getHandle(), reinterpret_cast<struct sockaddr*>(&address), &temp) != -1) {
            return ntohs(address.sin_port);
        }
    }

    // We failed to retrieve the port
    return 0;
}


////////////////////////////////////////////////////////////
Socket::Status TcpSocket::connect(const IpAddress& remoteAddress, unsigned short remotePort, std::chrono::seconds timeout) {
    // Create the internal socket if it doesn't exist
    create();

    // Create the remote address
    sockaddr_in address = Socket::createAddress(remoteAddress.toInteger(), remotePort);

    if ( timeout.count() < 0 ) {
        // ----- We're not using a timeout: just try to connect -----

        // Connect the socket
        if (::connect(getHandle(), reinterpret_cast<struct sockaddr*>(&address), sizeof(address)) == -1) {
            return Socket::getErrorStatus();
        }

        // Connection succeeded
        return Done;
    }
    else {
        // ----- We're using a timeout: we'll need a few tricks to make it work -----

        // Save the previous blocking state
        bool blocking = isBlocking();

        // Switch to non-blocking to enable our connection timeout
        if ( blocking ) {
            setBlocking( false );
        }

        // Try to connect to the remote address
        if (::connect(getHandle(), reinterpret_cast<sockaddr*>(&address), sizeof(address)) >= 0) {
            // We got instantly connected! (it may no happen a lot...)
            return Done;
        }

        // Get the error status
        Status status = Socket::getErrorStatus();

        // If we were in non-blocking mode, return immediatly
        if (!blocking) {
            return status;
        }

        // Otherwise, wait until something happens to our socket (success, timeout or error)
        if (status == Socket::NotReady) {
            // Setup the selector
            fd_set selector;
            FD_ZERO(&selector);
            FD_SET(getHandle(), &selector);

            // Setup the timeout
            timeval time;
            time.tv_sec = std::chrono::duration_cast<std::chrono::seconds>(timeout).count();
            time.tv_usec = std::chrono::duration_cast<std::chrono::microseconds>(timeout).count() % 1000000;

            // Wait for something to write on our socket (which means that the connection request has returned)
            if (select(static_cast<int>(getHandle() + 1), nullptr, &selector, nullptr, &time) > 0) {
                // At this point the connection may have been either accepted or refused.
                // To know whether it's a success or a failure, we must check the address of the connected peer
                if (getRemoteAddress() != sf::IpAddress::None) {
                    // Connection accepted
                    status = Done;
                }
                else {
                    // Connection refused
                    status = Socket::getErrorStatus();
                }
            }
            else {
                // Failed to connect before timeout is over
                status = Socket::getErrorStatus();
            }
        }

        // Switch back to blocking mode
        setBlocking( true );

        return status;
    }
}

void TcpSocket::disconnect() {
    // Close the socket
    close();

    // Reset the pending packet data
    m_pendingPacket = PendingPacket();
}

Socket::Status TcpSocket::send(const void* data, std::size_t size) {
    // Check the parameters
    if (!data || (size == 0)) {
        std::cerr << "Cannot send data over the network (no data to send)\n";
        return Error;
    }

    // Loop until every byte has been sent
    int sent = 0;
    int sizeToSend = static_cast<int>(size);
    for (int length = 0; length < sizeToSend; length += sent) {
        // Send a chunk of data
        sent = ::send(getHandle(), static_cast<const char*>(data) + length, sizeToSend - length, 0);

        // Check for errors
        if (sent < 0)
            return Socket::getErrorStatus();
    }

    return Done;
}


////////////////////////////////////////////////////////////
Socket::Status TcpSocket::receive(void* data, std::size_t size, std::size_t& received) {
    // First clear the variables to fill
    received = 0;

    // Check the destination buffer
    if (!data) {
        std::cerr << "Cannot receive data from the network (the destination buffer is invalid)\n";
        return Error;
    }

    // Receive a chunk of bytes
    int sizeReceived = recv(getHandle(), static_cast<char*>(data), static_cast<int>(size), 0);

    // Check the number of bytes received
    if (sizeReceived > 0) {
        received = static_cast<std::size_t>(sizeReceived);
        return Done;
    }
    else if (sizeReceived == 0) {
        return Socket::Disconnected;
    }
    else {
        return Socket::getErrorStatus();
    }
}

Socket::Status TcpSocket::send(Packet& packet) {
    // TCP is a stream protocol, it doesn't preserve messages boundaries.
    // This means that we have to send the packet size first, so that the
    // receiver knows the actual end of the packet in the data stream.

    // Get the data to send from the packet
    std::size_t size = 0;
    const void* data = packet.onSend(size);

    // First send the packet size
    uint32_t packetSize = htonl(static_cast<uint32_t>(size));
    Status status = send(reinterpret_cast<const char*>(&packetSize), sizeof(packetSize));

    // Make sure that the size was properly sent
    if (status != Done) {
        return status;
    }

    // Send the packet data
    if (packetSize > 0) {
        return send(data, size);
    }
    else {
        return Done;
    }
}

Socket::Status TcpSocket::receive(Packet& packet) {
    // First clear the variables to fill
    packet.clear();

    // We start by getting the size of the incoming packet
    uint32_t packetSize = 0;
    std::size_t received = 0;
    if (m_pendingPacket.SizeReceived < sizeof(m_pendingPacket.Size)) {
        // Loop until we've received the entire size of the packet
        // (even a 4 byte variable may be received in more than one call)
        while (m_pendingPacket.SizeReceived < sizeof(m_pendingPacket.Size)) {
            char* data = reinterpret_cast<char*>(&m_pendingPacket.Size) + m_pendingPacket.SizeReceived;
            Status status = receive(data, sizeof(m_pendingPacket.Size) - m_pendingPacket.SizeReceived, received);
            m_pendingPacket.SizeReceived += received;

            if (status != Done)
                return status;
        }

        // The packet size has been fully received
        packetSize = ntohl(m_pendingPacket.Size);
    }
    else {
        // The packet size has already been received in a previous call
        packetSize = ntohl(m_pendingPacket.Size);
    }

    // Loop until we receive all the packet data
    char buffer[1024];
    while (m_pendingPacket.Data.size() < packetSize) {
        // Receive a chunk of data
        std::size_t sizeToGet = std::min<size_t>(static_cast<std::size_t>(packetSize - m_pendingPacket.Data.size()), sizeof(buffer));
        Status status = receive(buffer, sizeToGet, received);
        if (status != Done) {
            return status;
        }

        // Append it into the packet
        if (received > 0) {
            m_pendingPacket.Data.resize(m_pendingPacket.Data.size() + received);
            char* begin = &m_pendingPacket.Data[0] + m_pendingPacket.Data.size() - received;
            std::memcpy(begin, buffer, received);
        }
    }

    // We have received all the packet data: we can copy it to the user packet
    if (!m_pendingPacket.Data.empty()) {
        packet.onReceive(&m_pendingPacket.Data[0], m_pendingPacket.Data.size());
    }

    // Clear the pending packet data
    m_pendingPacket = PendingPacket();

    return Done;
}

TcpSocket::PendingPacket::PendingPacket() :
Size        (0),
SizeReceived(0),
Data        () {

}

} // namespace sf