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

/* !!! THIS IS AN EXTREMELY ALTERED AND PURPOSE-BUILT VERSION OF SFML !!!
 * This distribution is designed to possess only a limited subset of the
 * original library's functionality and to only build on VxWorks 6.3.
 * The original distribution of this software has many more features and
 * supports more platforms.
 */

#ifndef SFML_TCPSOCKET_HPP
#define SFML_TCPSOCKET_HPP

#include "../../SFMLNetwork/Socket.hpp"

#include <chrono>
#include <cstdint>


namespace sf {
class TcpListener;
class IpAddress;
class Packet;

////////////////////////////////////////////////////////////
/// \brief Specialized socket using the TCP protocol
///
////////////////////////////////////////////////////////////
class TcpSocket : public Socket {
public:

    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    ////////////////////////////////////////////////////////////
    TcpSocket();

    ////////////////////////////////////////////////////////////
    /// \brief Get the port to which the socket is bound locally
    ///
    /// If the socket is not connected, this function returns 0.
    ///
    /// \return Port to which the socket is bound
    ///
    /// \see connect, getRemotePort
    ///
    ////////////////////////////////////////////////////////////
    unsigned short getLocalPort() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the address of the connected peer
    ///
    /// It the socket is not connected, this function returns
    /// sf::IpAddress::None.
    ///
    /// \return Address of the remote peer
    ///
    /// \see getRemotePort
    ///
    ////////////////////////////////////////////////////////////
    IpAddress getRemoteAddress() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the port of the connected peer to which
    ///        the socket is connected
    ///
    /// If the socket is not connected, this function returns 0.
    ///
    /// \return Remote port to which the socket is connected
    ///
    /// \see getRemoteAddress
    ///
    ////////////////////////////////////////////////////////////
    unsigned short getRemotePort() const;

    ////////////////////////////////////////////////////////////
    /// \brief Connect the socket to a remote peer
    ///
    /// In blocking mode, this function may take a while, especially
    /// if the remote peer is not reachable. The last parameter allows
    /// you to stop trying to connect after a given timeout.
    /// If the socket was previously connected, it is first disconnected.
    ///
    /// \param remoteAddress Address of the remote peer
    /// \param remotePort    Port of the remote peer
    /// \param timeout       Optional maximum time to wait
    ///
    /// \return Status code
    ///
    /// \see disconnect
    ///
    ////////////////////////////////////////////////////////////
    Status connect( const IpAddress& remoteAddress ,
                    unsigned short remotePort ,
                    std::chrono::seconds timeout = std::chrono::seconds(0) );

    ////////////////////////////////////////////////////////////
    /// \brief Disconnect the socket from its remote peer
    ///
    /// This function gracefully closes the connection. If the
    /// socket is not connected, this function has no effect.
    ///
    /// \see connect
    ///
    ////////////////////////////////////////////////////////////
    void disconnect();

    ////////////////////////////////////////////////////////////
    /// \brief Send raw data to the remote peer
    ///
    /// This function will fail if the socket is not connected.
    ///
    /// \param data Pointer to the sequence of bytes to send
    /// \param size Number of bytes to send
    ///
    /// \return Status code
    ///
    /// \see receive
    ///
    ////////////////////////////////////////////////////////////
    Status send( const void* data , std::size_t size );

    ////////////////////////////////////////////////////////////
    /// \brief Receive raw data from the remote peer
    ///
    /// In blocking mode, this function will wait until some
    /// bytes are actually received.
    /// This function will fail if the socket is not connected.
    ///
    /// \param data     Pointer to the array to fill with the received bytes
    /// \param size     Maximum number of bytes that can be received
    /// \param received This variable is filled with the actual number of bytes received
    ///
    /// \return Status code
    ///
    /// \see send
    ///
    ////////////////////////////////////////////////////////////
    Status receive( void* data , std::size_t size , std::size_t& received );

    ////////////////////////////////////////////////////////////
    /// \brief Send a formatted packet of data to the remote peer
    ///
    /// This function will fail if the socket is not connected.
    ///
    /// \param packet Packet to send
    ///
    /// \return Status code
    ///
    /// \see receive
    ///
    ////////////////////////////////////////////////////////////
    Status send( Packet& packet );

    ////////////////////////////////////////////////////////////
    /// \brief Receive a formatted packet of data from the remote peer
    ///
    /// In blocking mode, this function will wait until the whole packet
    /// has been received.
    /// This function will fail if the socket is not connected.
    ///
    /// \param packet Packet to fill with the received data
    ///
    /// \return Status code
    ///
    /// \see send
    ///
    ////////////////////////////////////////////////////////////
    Status receive( Packet& packet );

private:

    friend class TcpListener;

    ////////////////////////////////////////////////////////////
    /// \brief Structure holding the data of a pending packet
    ///
    ////////////////////////////////////////////////////////////
    struct PendingPacket {
        PendingPacket();

        uint32_t Size;         ///< Data of packet size
        std::size_t SizeReceived; ///< Number of size bytes received so far
        std::vector<char> Data;         ///< Data of the packet
    };

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    PendingPacket m_pendingPacket; ///< Temporary data of the packet currently being received
};
} // namespace sf


#endif // SFML_TCPSOCKET_HPP

