////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2012 Laurent Gomila (laurent.gom@gmail.com)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the
// use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely,
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

#include "../SFML/Network/IpAddress.hpp"
#include "Socket.hpp"

namespace {
uint32_t resolve(const std::string& address) {
    if (address == "255.255.255.255") {
        // The broadcast address needs to be handled explicitly,
        // because it is also the value returned by inet_addr on error
        return INADDR_BROADCAST;
    } else {
        // Try to convert the address as a byte representation
        // ("xxx.xxx.xxx.xxx")
        uint32_t ip = inet_addr(const_cast<char*>(address.c_str()));
        if (ip != INADDR_NONE) {
            return ip;
        }

        // Not a valid address, try to convert it as a host name
        hostent* host = gethostbyname(const_cast<char*>(address.c_str()));
        if (host) {
            return reinterpret_cast<in_addr*>(host->h_addr)->s_addr;
        }

        // Not a valid address nor a host name
        return 0;
    }
}
}  // namespace

namespace sf {
const IpAddress IpAddress::None(0, 0, 0, 0);
const IpAddress IpAddress::LocalHost(127, 0, 0, 1);
const IpAddress IpAddress::Broadcast(255, 255, 255, 255);

IpAddress::IpAddress(const std::string& address)
    : m_address(resolve(address)) {}

IpAddress::IpAddress(const char* address) : m_address(resolve(address)) {}

IpAddress::IpAddress(uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3)
    : m_address(htonl((byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3)) {}

IpAddress::IpAddress(uint32_t address) : m_address(htonl(address)) {}

std::string IpAddress::toString() const {
    in_addr address;
    address.s_addr = m_address;

    return inet_ntoa(address);
}

uint32_t IpAddress::toInteger() const { return ntohl(m_address); }

IpAddress IpAddress::getLocalAddress() {
    // The method here is to connect a UDP socket to anyone (here to localhost),
    // and get the local socket address with the getsockname function.
    // UDP connection will not send anything to the network, so this function
    // won't cause any overhead.

    IpAddress localAddress;

    // Create the socket
    int sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        return localAddress;
    }

    // Connect the socket to localhost on any port
    sockaddr_in address = Socket::createAddress(ntohl(INADDR_LOOPBACK), 0);
    if (connect(sock, reinterpret_cast<sockaddr*>(&address), sizeof(address)) ==
        -1) {
        ::close(sock);
        return localAddress;
    }

    // Get the local address of the socket connection
    Socket::AddrLength size = sizeof(address);
    socklen_t temp = size;
    if (getsockname(sock, reinterpret_cast<sockaddr*>(&address), &temp) == -1) {
        ::close(sock);
        return localAddress;
    }

    // Close the socket
    ::close(sock);

    // Finally build the IP address
    localAddress = IpAddress(ntohl(address.sin_addr.s_addr));

    return localAddress;
}

bool operator==(const IpAddress& left, const IpAddress& right) {
    return left.toInteger() == right.toInteger();
}

bool operator!=(const IpAddress& left, const IpAddress& right) {
    return !(left == right);
}

bool operator<(const IpAddress& left, const IpAddress& right) {
    return left.toInteger() < right.toInteger();
}

bool operator>(const IpAddress& left, const IpAddress& right) {
    return right < left;
}

bool operator<=(const IpAddress& left, const IpAddress& right) {
    return !(right < left);
}

bool operator>=(const IpAddress& left, const IpAddress& right) {
    return !(left < right);
}

std::istream& operator>>(std::istream& stream, IpAddress& address) {
    std::string str;
    stream >> str;
    address = IpAddress(str);

    return stream;
}

std::ostream& operator<<(std::ostream& stream, const IpAddress& address) {
    return stream << address.toString();
}
}  // namespace sf
