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

#ifndef SFML_IPADDRESS_HPP
#define SFML_IPADDRESS_HPP

#include <istream>
#include <ostream>
#include <string>


namespace sf {
class IpAddress {
public:
    IpAddress();

    /* The address can be either a decimal address (192.168.1.56) or a network
     * name ("localhost")
     */
    IpAddress(const std::string& address);
    IpAddress(const char* address);

    /* Construct the address from 4 bytes
     *
     * Calling IpAddress(a, b, c, d) is equivalent to calling
     * IpAddress("a.b.c.d"), but safer as it doesn't have to
     * parse a string to get the address components.
     */
    IpAddress(uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3);

    ////////////////////////////////////////////////////////////
    /// \brief Construct the address from a 32-bits integer
    ///
    /// This constructor uses the internal representation of
    /// the address directly. It should be used for optimization
    /// purposes, and only if you got that representation from
    /// IpAddress::ToInteger().
    ///
    /// \param address 4 bytes of the address packed into a 32-bits integer
    ///
    /// \see toInteger
    ///
    ////////////////////////////////////////////////////////////
    explicit IpAddress(uint32_t address);

    ////////////////////////////////////////////////////////////
    /// \brief Get a string representation of the address
    ///
    /// The returned string is the decimal representation of the
    /// IP address (like "192.168.1.56"), even if it was constructed
    /// from a host name.
    ///
    /// \return String representation of the address
    ///
    /// \see toInteger
    ///
    ////////////////////////////////////////////////////////////
    std::string toString() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get an integer representation of the address
    ///
    /// The returned number is the internal representation of the
    /// address, and should be used for optimization purposes only
    /// (like sending the address through a socket).
    /// The integer produced by this function can then be converted
    /// back to a sf::IpAddress with the proper constructor.
    ///
    /// \return 32-bits unsigned integer representation of the address
    ///
    /// \see toString
    ///
    ////////////////////////////////////////////////////////////
    uint32_t toInteger() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the computer's local address
    ///
    /// The local address is the address of the computer from the
    /// LAN point of view, i.e. something like 192.168.1.56. It is
    /// meaningful only for communications over the local network.
    /// Unlike getPublicAddress, this function is fast and may be
    /// used safely anywhere.
    ///
    /// \return Local IP address of the computer
    ///
    /// \see getPublicAddress
    ///
    ////////////////////////////////////////////////////////////
    static IpAddress getLocalAddress();

    static const IpAddress None;      ///< Value representing an empty/invalid address
    static const IpAddress LocalHost; ///< The "localhost" address (for connecting a computer to itself locally)
    static const IpAddress Broadcast; ///< The "broadcast" address (for sending UDP messages to everyone on a local network)

private:
    uint32_t m_address;
};

bool operator ==(const IpAddress& left, const IpAddress& right);

bool operator !=(const IpAddress& left, const IpAddress& right);

bool operator <(const IpAddress& left, const IpAddress& right);

bool operator >(const IpAddress& left, const IpAddress& right);

bool operator <=(const IpAddress& left, const IpAddress& right);

bool operator >=(const IpAddress& left, const IpAddress& right);

std::istream& operator >>(std::istream& stream, IpAddress& address);

std::ostream& operator <<(std::ostream& stream, const IpAddress& address);

} // namespace sf


#endif // SFML_IPADDRESS_HPP
