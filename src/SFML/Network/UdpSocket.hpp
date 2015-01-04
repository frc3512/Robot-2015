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

#ifndef SFML_UDPSOCKET_HPP
#define SFML_UDPSOCKET_HPP

#include "../../SFMLNetwork/Socket.hpp"
#include <vector>


namespace sf
{
class IpAddress;
class Packet;

////////////////////////////////////////////////////////////
/// \brief Specialized socket using the UDP protocol
///
////////////////////////////////////////////////////////////
class UdpSocket : public Socket
{
public :

    ////////////////////////////////////////////////////////////
    // Constants
    ////////////////////////////////////////////////////////////
    enum
    {
        MaxDatagramSize = 65507 ///< The maximum number of bytes that can be sent in a single UDP datagram
    };

    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    ////////////////////////////////////////////////////////////
    UdpSocket();

    ////////////////////////////////////////////////////////////
    /// \brief Get the port to which the socket is bound locally
    ///
    /// If the socket is not bound to a port, this function
    /// returns 0.
    ///
    /// \return Port to which the socket is bound
    ///
    /// \see bind
    ///
    ////////////////////////////////////////////////////////////
    unsigned short getLocalPort() const;

    ////////////////////////////////////////////////////////////
    /// \brief Bind the socket to a specific port
    ///
    /// Binding the socket to a port is necessary for being
    /// able to receive data on that port.
    /// You can use the special value Socket::AnyPort to tell the
    /// system to automatically pick an available port, and then
    /// call getLocalPort to retrieve the chosen port.
    ///
    /// \param port Port to bind the socket to
    ///
    /// \return Status code
    ///
    /// \see unbind, getLocalPort
    ///
    ////////////////////////////////////////////////////////////
    Status bind(unsigned short port);

    ////////////////////////////////////////////////////////////
    /// \brief Unbind the socket from the local port to which it is bound
    ///
    /// The port that the socket was previously using is immediately
    /// available after this function is called. If the
    /// socket is not bound to a port, this function has no effect.
    ///
    /// \see bind
    ///
    ////////////////////////////////////////////////////////////
    void unbind();

    ////////////////////////////////////////////////////////////
    /// \brief Send raw data to a remote peer
    ///
    /// Make sure that \a size is not greater than
    /// UdpSocket::MaxDatagramSize, otherwise this function will
    /// fail and no data will be sent.
    ///
    /// \param data          Pointer to the sequence of bytes to send
    /// \param size          Number of bytes to send
    /// \param remoteAddress Address of the receiver
    /// \param remotePort    Port of the receiver to send the data to
    ///
    /// \return Status code
    ///
    /// \see receive
    ///
    ////////////////////////////////////////////////////////////
    Status send(const void* data, std::size_t size, const IpAddress& remoteAddress, unsigned short remotePort);

    ////////////////////////////////////////////////////////////
    /// \brief Receive raw data from a remote peer
    ///
    /// In blocking mode, this function will wait until some
    /// bytes are actually received.
    /// Be careful to use a buffer which is large enough for
    /// the data that you intend to receive, if it is too small
    /// then an error will be returned and *all* the data will
    /// be lost.
    ///
    /// \param data          Pointer to the array to fill with the received bytes
    /// \param size          Maximum number of bytes that can be received
    /// \param received      This variable is filled with the actual number of bytes received
    /// \param remoteAddress Address of the peer that sent the data
    /// \param remotePort    Port of the peer that sent the data
    ///
    /// \return Status code
    ///
    /// \see send
    ///
    ////////////////////////////////////////////////////////////
    Status receive(void* data, std::size_t size, std::size_t& received, IpAddress& remoteAddress, unsigned short& remotePort);

    ////////////////////////////////////////////////////////////
    /// \brief Send a formatted packet of data to a remote peer
    ///
    /// Make sure that the packet size is not greater than
    /// UdpSocket::MaxDatagramSize, otherwise this function will
    /// fail and no data will be sent.
    ///
    /// \param packet        Packet to send
    /// \param remoteAddress Address of the receiver
    /// \param remotePort    Port of the receiver to send the data to
    ///
    /// \return Status code
    ///
    /// \see receive
    ///
    ////////////////////////////////////////////////////////////
    Status send(Packet& packet, const IpAddress& remoteAddress, unsigned short remotePort);

    ////////////////////////////////////////////////////////////
    /// \brief Receive a formatted packet of data from a remote peer
    ///
    /// In blocking mode, this function will wait until the whole packet
    /// has been received.
    ///
    /// \param packet        Packet to fill with the received data
    /// \param remoteAddress Address of the peer that sent the data
    /// \param remotePort    Port of the peer that sent the data
    ///
    /// \return Status code
    ///
    /// \see send
    ///
    ////////////////////////////////////////////////////////////
    Status receive(Packet& packet, IpAddress& remoteAddress, unsigned short& remotePort);

private:

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    std::vector<char> m_buffer; ///< Temporary buffer holding the received data in Receive(Packet)
};

} // namespace sf


#endif // SFML_UDPSOCKET_HPP
