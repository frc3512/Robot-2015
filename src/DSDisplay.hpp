// Copyright (c) FRC Team 3512, Spartatroniks 2015-2017. All Rights Reserved.

#pragma once

/* This class allows you to pack data into an SFML packet and send it to an
 * application on the DriverStation that displays it in a GUI.
 *
 * USAGE:
 * 1) Call DSDisplay::getInstance() to create an instance of this
 *    class. The port number passed in should be the port on which
 *    communications will be received (probably 1130).
 * 2) Call clear() on the pointer to empty the packet before adding new data.
 * 3) Add new data with the << operator (e.g. dsDisplay << 4.f; dsDisplay <<
 *    myVar;).
 * 4) After all data is packed, call sendToDS() to send the data to the Driver
 *    Station.
 *
 * Extract the received packet on the DriverStation with the >> operator like
 * any other SFML packet.
 *
 * receiveFromDS() requires that the file GUISettings.txt exist in
 * "/c", which follows the convention described in the
 * DSDisplay's readme. This class creates a file "autonMode.txt"
 * internally to store the currently selected autonomous routine.
 *
 * Before sending HUD data to the DriverStation, call clear() followed by
 * calls to addElementData() and a call to sendToDS(). If clear() isn't
 * called first, undefined behavior may result. (The header "display\r\n" isn't
 * inserted when the packet isn't empty.)
 *
 * Note: It doesn't matter in which order the data in the received packet is
 *       extracted in the application on the Driver Station.
 *
 * The packets are always sent to 10.35.12.42 for testing purposes
 */

#include <stdint.h>

#include <string>

#include "AutonContainer.hpp"
#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/Packet.hpp"
#include "SFML/Network/UdpSocket.hpp"

class DSDisplay {
public:
    enum StatusLight : int8_t { active, standby, inactive };

    static DSDisplay& GetInstance(uint16_t dsPort);

    // Empties internal packet of data
    void Clear();

    // Sends data currently in class's internal packet to Driver Station
    void SendToDS();

    // Receives control commands from Driver Station and processes them
    const std::string ReceiveFromDS();

    // Add and remove autonomous functions
    template <class T>
    void AddAutoMethod(const std::string& methodName, void (T::*function)(),
                       T* object);
    void DeleteAllMethods();

    // Runs autonomous function currently selected
    void ExecAutonomous();

    // Returns position of currently selected autonomous in function array
    char GetAutonID() const;

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
    void AddData(std::string ID, StatusLight data);
    void AddData(std::string ID, bool data);
    void AddData(std::string ID, int8_t data);
    void AddData(std::string ID, int32_t data);
    void AddData(std::string ID, uint32_t data);
    void AddData(std::string ID, std::string data);
    void AddData(std::string ID, float data);
    void AddData(std::string ID, double data);

private:
    explicit DSDisplay(uint16_t portNumber);

    DSDisplay(const DSDisplay&) = delete;
    DSDisplay& operator=(const DSDisplay&) = delete;

    sf::Packet m_packet;

    sf::UdpSocket m_socket;  // socket for sending data to Driver Station
    sf::IpAddress m_dsIP{sf::IpAddress::None};  // IP address of Driver Station
    uint16_t m_dsPort;                          // port to which to send data

    // Stores IP address temporarily during receive
    sf::IpAddress m_recvIP{0, 0, 0, 0};

    // Stores port temporarily during receive
    uint16_t m_recvPort = 0;

    // Buffer for Driver Station requests
    char m_recvBuffer[256];

    // Holds number of bytes received from Driver Station
    size_t m_recvAmount = 0;

    AutonContainer m_autonModes;
    char m_curAutonMode;
};

#include "DSDisplay.inl"
