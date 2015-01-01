//=============================================================================
//File Name: DriverStationDisplay.hpp
//Description: Receives IP address from remote host then sends HUD data there
//Author: FRC Team 3512, Spartatroniks
//=============================================================================

#ifndef DRIVER_STATION_DISPLAY_HPP
#define DRIVER_STATION_DISPLAY_HPP

/* This class allows you to pack data into an SFML packet and send it to an
 * application on the DriverStation that displays it in a GUI.
 *
 * USAGE:
 * 1) Call DriverStationDisplay::getInstance() to create an instance of this
 *    class. The port number passed in should be the port on which
 *    communications will be received (probably 1130).
 * 2) Call clear() on the pointer to empty the packet before adding new data.
 * 3) Add new data with the << operator (e.g. *dsPtr << 4.f; *dsPtr << myVar;).
 * 4) After all data is packed, call sendToDS() to send the data to the Driver
 *    Station.
 *
 * Extract the received packet on the DriverStation with the >> operator like
 * any other SFML packet.
 *
 * receiveFromDS() requires that the file GUISettings.txt exist in
 * "/c", which follows the convention described in the
 * DriverStationDisplay's readme. This class creates a file "autonMode.txt"
 * internally to store the currently selected autonomous routine.
 *
 * Before sending HUD data to the DriverStation, call clear() followed by
 * calls to DS::AddElementData() and a call to sendToDS(). If clear() isn't
 * called first, undefined behavior may result. (The header "display\r\n" isn't
 * inserted when the packet isn't empty.)
 *
 * Note: It doesn't matter in which order the data in the received packet is
 *       extracted in the application on the Driver Station.
 *
 * The packets are always sent to 10.35.12.42 for testing purposes
 */

#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/Packet.hpp"
#include "SFML/Network/UdpSocket.hpp"

#include "AutonContainer.hpp"

#include <cstring>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <string>

template <class T>
class DriverStationDisplayInit {
public:
    DriverStationDisplayInit();
    ~DriverStationDisplayInit();
};

template <class T>
class DriverStationDisplay : public sf::Packet {
public:
    virtual ~DriverStationDisplay();

    static DriverStationDisplay<T>* getInstance( unsigned short dsPort );

    // Empties internal packet of data
    void clear();

    /* Sends data currently in packet to Driver Station. 'userData' holds the
     * packet to be sent to the Driver Station. If the pointer is NULL, this
     * class's internal packet is sent instead.
     */
    void sendToDS( sf::Packet* userData = NULL );

    // Receives control commands from Driver Station and processes them
    const std::string receiveFromDS();

    // Add and remove autonomous functions
    void addAutonMethod( const std::string& methodName , void ( T::*function)() , T* object );
    void deleteAllMethods();

    // Runs autonomous function currently selected
    void execAutonomous();

private:
    DriverStationDisplay( unsigned short portNumber );

    DriverStationDisplay( const DriverStationDisplay<T>& );
    DriverStationDisplay<T>& operator=( const DriverStationDisplay<T>& );

    static DriverStationDisplay<T>* m_dsDisplay;

    sf::IpAddress m_dsIP; // IP address of Driver Station
    unsigned short m_dsPort; // port to which to send data

    sf::IpAddress m_recvIP; // stores IP address temporarily during receive
    unsigned short m_recvPort; // stores port temporarily during receive
    char m_recvBuffer[256]; // buffer for Driver Station requests
    size_t m_recvAmount; // holds number of bytes received from Driver Station

    AutonContainer<T> m_autonModes;
    char curAutonMode;

    // Allows class to clean itself up when the robot task exits
    friend class DriverStationDisplayInit<T>;
    static DriverStationDisplayInit<T> deleter;
};

namespace DS {
    enum StatusLight {
        active,
        standby,
        inactive
    };

    /* Add UI element data to packet
     *
     * The types allowed for 'data' are char, int, unsigned int, std::wstring,
     * std::string, float, and double. String literals are converted to
     * std::string implicitly. Every std::string is converted to a std::wstring
     * before packing the string in the packet.
     *
     * The correct identifier to send with the data is deduced from its type at
     * compile time. floats and doubles are converted to strings because VxWorks
     * messes up floats over the network.
     */
    template <class T, class U>
    void AddElementData( DriverStationDisplay<T>* inst , std::string ID , U data );
}

class SocketInit {
public:
    SocketInit();
    virtual ~SocketInit();

    static sf::UdpSocket& getInstance( unsigned short portNumber = 0 );

private:
    static sf::UdpSocket* m_socket;
};

#include "DriverStationDisplay.inl"

#endif // DRIVER_STATION_DISPLAY_HPP
