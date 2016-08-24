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
////////////////////////////////////////////////////////////

/* !!! THIS IS AN EXTREMELY ALTERED AND PURPOSE-BUILT VERSION OF SFML !!!
 * This distribution is designed to possess only a limited subset of the
 * original library's functionality and to only build on VxWorks 6.3.
 * The original distribution of this software has many more features and
 * supports more platforms.
 */

#ifndef SFML_PACKET_HPP
#define SFML_PACKET_HPP

#include <string>

namespace std {
typedef basic_string<wchar_t> wstring;
}

#include <cstdint>
#include <vector>

uint64_t htonll(uint64_t value);
uint64_t ntohll(uint64_t value);

namespace sf {
class TcpSocket;
class UdpSocket;

////////////////////////////////////////////////////////////
/// \brief Utility class to build blocks of data to transfer
///        over the network
///
////////////////////////////////////////////////////////////
class Packet {
    // A bool-like type that cannot be converted to integer or pointer types
    typedef bool (Packet::*BoolType)(std::size_t);

public:
    Packet() = default;

    virtual ~Packet() = default;

    // Append data to the end of the packet
    void append(const void* data, std::size_t sizeInBytes);

    // Empty the packet
    void clear();

    /* Get a pointer to the data contained in the packet
     *
     * Warning: the returned pointer may become invalid after you append data
     * to the packet, therefore it should never be stored.
     *
     * The return pointer is nullptr if the packet is empty.
     */
    const void* getData() const;

    /* Get the size of the data contained in the packet in bytes
     *
     * This function returns the number of bytes pointed to by what getData
     * returns.
     */
    std::size_t getDataSize() const;

    // Returns 'true' if reading position has reached end of the packet
    bool endOfPacket() const;

public:
    ////////////////////////////////////////////////////////////
    /// \brief Test the validity of the packet, for reading
    ///
    /// This operator allows to test the packet as a boolean
    /// variable, to check if a reading operation was successful.
    ///
    /// A packet will be in an invalid state if it has no more
    /// data to read.
    ///
    /// This behavior is the same as standard C++ streams.
    ///
    /// Usage example:
    /// \code
    /// float x;
    /// packet >> x;
    /// if (packet)
    /// {
    ///    // ok, x was extracted successfully
    /// }
    ///
    /// // -- or --
    ///
    /// float x;
    /// if (packet >> x)
    /// {
    ///    // ok, x was extracted successfully
    /// }
    /// \endcode
    ///
    /// Don't focus on the return type, it's equivalent to bool but
    /// it disallows unwanted implicit conversions to integer or
    /// pointer types.
    ///
    /// \return True if last data extraction from packet was successful
    ///
    /// \see endOfPacket
    ///
    ////////////////////////////////////////////////////////////
    operator BoolType() const;

    // Overloads of operator >> to read data from the packet
    Packet& operator>>(bool& data);
    Packet& operator>>(int8_t& data);
    Packet& operator>>(uint8_t& data);
    Packet& operator>>(int16_t& data);
    Packet& operator>>(uint16_t& data);
    Packet& operator>>(int32_t& data);
    Packet& operator>>(uint32_t& data);
    Packet& operator>>(int64_t& data);
    Packet& operator>>(uint64_t& data);
    Packet& operator>>(float& data);
    Packet& operator>>(double& data);
    Packet& operator>>(char* data);
    Packet& operator>>(std::string& data);
    Packet& operator>>(wchar_t* data);
    Packet& operator>>(std::wstring& data);

    // Overloads of operator << to write data into the packet
    Packet& operator<<(bool data);
    Packet& operator<<(int8_t data);
    Packet& operator<<(uint8_t data);
    Packet& operator<<(int16_t data);
    Packet& operator<<(uint16_t data);
    Packet& operator<<(int32_t data);
    Packet& operator<<(uint32_t data);
    Packet& operator<<(int64_t data);
    Packet& operator<<(uint64_t data);
    Packet& operator<<(float data);
    Packet& operator<<(double data);
    Packet& operator<<(const char* data);
    Packet& operator<<(const std::string& data);
    Packet& operator<<(const wchar_t* data);
    Packet& operator<<(const std::wstring& data);

protected:
    friend class TcpSocket;
    friend class UdpSocket;

    ////////////////////////////////////////////////////////////
    /// \brief Called before the packet is sent over the network
    ///
    /// This function can be defined by derived classes to
    /// transform the data before it is sent; this can be
    /// used for compression, encryption, etc.
    /// The function must return a pointer to the modified data,
    /// as well as the number of bytes pointed.
    /// The default implementation provides the packet's data
    /// without transforming it.
    ///
    /// \param size Variable to fill with the size of data to send
    ///
    /// \return Pointer to the array of bytes to send
    ///
    /// \see onReceive
    ///
    ////////////////////////////////////////////////////////////
    virtual const void* onSend(std::size_t& size);

    ////////////////////////////////////////////////////////////
    /// \brief Called after the packet is received over the network
    ///
    /// This function can be defined by derived classes to
    /// transform the data after it is received; this can be
    /// used for uncompression, decryption, etc.
    /// The function receives a pointer to the received data,
    /// and must fill the packet with the transformed bytes.
    /// The default implementation fills the packet directly
    /// without transforming the data.
    ///
    /// \param data Pointer to the received bytes
    /// \param size Number of bytes
    ///
    /// \see onSend
    ///
    ////////////////////////////////////////////////////////////
    virtual void onReceive(const void* data, std::size_t size);

private:
    // Disallow comparisons between packets
    bool operator==(const Packet& right) const;
    bool operator!=(const Packet& right) const;

    /* Returns 'true' if the packet can extract a given number of bytes
     *
     * This function updates accordingly the state of the packet.
     */
    bool checkSize(std::size_t size);

    std::vector<char> m_packetData;  ///< Data stored in the packet
    std::size_t m_readPos = 0;       ///< Current reading position in the packet
    bool m_isValid = true;           ///< Reading state of the packet
};
}  // namespace sf

#endif  // SFML_PACKET_HPP
