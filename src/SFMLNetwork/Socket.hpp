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

#ifndef SFML_SOCKET_HPP
#define SFML_SOCKET_HPP

#include "../SFML/System/NonCopyable.hpp"
#include <vector>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>


namespace sf {
class SocketSelector;

////////////////////////////////////////////////////////////
/// \brief Base class for all the socket types
///
////////////////////////////////////////////////////////////
class Socket : NonCopyable {
public:
    friend class IpAddress;

    typedef socklen_t AddrLength;

    // Status codes that may be returned by socket functions
    enum Status {
        Done ,         // The socket has sent / received the data
        NotReady ,     // The socket is not ready to send / receive data yet
        Disconnected , // The TCP socket has been disconnected
        Error         // An unexpected error happened
    };

    // Some special values used by sockets
    enum {
        AnyPort = 0 // Special value that tells the system to pick any available port
    };

    virtual ~Socket();

    /* Set the blocking state of the socket
     *
     * In blocking mode, calls will not return until they have completed their
     * task. For example, a call to Receive in blocking mode won't return until
     * some data was actually received. In non-blocking mode, calls will always
     * return immediately, using the return code to signal whether there was
     * data available or not. By default, all sockets are blocking.
     *
     * 'true' = blocking
     * 'false = non-blocking
     */
    void setBlocking( bool blocking );

    /* Tell whether the socket is in blocking or non-blocking mode
     * Returns 'true' if the socket is blocking, 'false' otherwise
     */
    bool isBlocking() const;

protected:
    // Types of protocols that the socket can use
    enum Type {
        Tcp , // TCP protocol
        Udp  // UDP protocol
    };

    // This constructor can only be accessed by derived classes.
    Socket( Type sockType );

    // Return the handle of the socket
    int getHandle() const;

    // Create the internal representation of the socket
    void create();

    // Create the internal representation of the socket from a socket handle
    void create( int handle );

    // Close the socket gracefully
    void close();

    /* Create an internal sockaddr_in address
     * Params:
     *     address Target address
     *     port    Target port
     * Returns sockaddr_in ready to be used by socket functions
     */
    static sockaddr_in createAddress( uint32_t address , unsigned short port );

    // Returns status corresponding to the last socket error
    static Socket::Status getErrorStatus();

private:
    friend class SocketSelector;

    Type m_socketType;
    int m_socket;      // Socket descriptor
    bool m_isBlocking; // Current blocking mode of the socket
};
} // namespace sf


#endif // SFML_SOCKET_HPP

