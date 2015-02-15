// =============================================================================
// File Name: DSDisplay.cpp
// Description: Receives IP address from remote host then sends HUD data there
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include <DSDisplay.hpp>

DSDisplay::~DSDisplay() {
}

DSDisplay& DSDisplay::getInstance(unsigned short dsPort) {
    static DSDisplay dsDisplay(dsPort);
    return dsDisplay;
}

void DSDisplay::clear() {
    sf::Packet::clear();
}

void DSDisplay::sendToDS(sf::Packet* userData) {
    if (m_dsIP != sf::IpAddress::None) {
        if (userData == nullptr) {
            m_socket.send(*static_cast<sf::Packet*>(this),
                          m_dsIP,
                          m_dsPort);
        }
        else {
            m_socket.send(*userData, m_dsIP, m_dsPort);
        }
    }

    // Used for testing purposes
    sf::IpAddress testIP(10, 35, 12, 42);
    if (userData == nullptr) {
        m_socket.send(*static_cast<sf::Packet*>(this),
                      testIP,
                      m_dsPort);
    }
    else {
        m_socket.send(*userData, testIP, m_dsPort);
    }
}

const std::string DSDisplay::receiveFromDS() {
    if (m_socket.receive(m_recvBuffer, 256, m_recvAmount,
                         m_recvIP,
                         m_recvPort) == sf::Socket::Done) {
        if (std::strncmp(m_recvBuffer, "connect\r\n", 9) == 0) {
            m_dsIP = m_recvIP;
            m_dsPort = m_recvPort;

            // Send GUI element file to DS
            clear();

            *this << static_cast<std::string>("guiCreate\r\n");

            // Open the file
            // FIXME: May crash if file has \n for newlines instead of \r\n
            std::ifstream guiFile("GUISettings.txt", std::ifstream::binary);

            if (guiFile.is_open()) {
                // Get its length
                guiFile.seekg(0, guiFile.end);
                unsigned int fileSize = guiFile.tellg();
                guiFile.seekg(0, guiFile.beg);

                // Send the length
                *this << static_cast<uint32_t>(fileSize);

                // Allocate a buffer for the file
                char* tempBuf = new char[fileSize];

                // Send the data TODO: htonl() the data before it's sent
                guiFile.read(tempBuf, fileSize);
                append(tempBuf, fileSize);

                delete[] tempBuf;
                guiFile.close();
            }

            sendToDS();

            // Send a list of available autonomous modes
            clear();

            *this << static_cast<std::string>("autonList\r\n");

            for (unsigned int i = 0; i < m_autonModes.size(); i++) {
                *this << m_autonModes.name(i);
            }

            sendToDS();

            // Make sure driver knows which autonomous mode is selected
            clear();

            *this << static_cast<std::string>("autonConfirmed\r\n");
            *this << m_autonModes.name(curAutonMode);

            sendToDS();

            return "connect\r\n";
        }
        else if (std::strncmp(m_recvBuffer, "autonSelect\r\n", 13) == 0) {
            // Next byte after command is selection choice
            curAutonMode = m_recvBuffer[13];

            clear();

            *this << static_cast<std::string>("autonConfirmed\r\n");
            *this << m_autonModes.name(curAutonMode);

            // Store newest autonomous choice to file for persistent storage
            std::ofstream autonModeFile("autonMode.txt",
                                        std::ofstream::trunc);
            if (autonModeFile.is_open()) {
                autonModeFile << curAutonMode;

                autonModeFile.close();
            }

            sendToDS();

            return "autonSelect\r\n";
        }
    }

    return "NONE";
}

DSDisplay::DSDisplay(unsigned short portNumber) :
    m_dsIP(sf::IpAddress::None),
    m_dsPort(portNumber) {
    m_socket.bind(portNumber);
    m_socket.setBlocking(false);

    m_recvIP = sf::IpAddress(0, 0, 0, 0);
    m_recvPort = 0;
    m_recvAmount = 0;

    // Retrieve stored autonomous index
    std::ifstream autonModeFile("autonMode.txt");
    if (autonModeFile.is_open()) {
        autonModeFile >> curAutonMode;

        autonModeFile.close();
    }
    else {
        curAutonMode = 0;
    }
}

void DSDisplay::deleteAllMethods() {
    m_autonModes.deleteAllMethods();
}

void DSDisplay::execAutonomous() {
    m_autonModes.execAutonomous(curAutonMode);
}

void DSDisplay::addData(std::string ID, StatusLight data) {
    // If packet is empty, add "display\r\n" header to packet
    if (sf::Packet::getData() == nullptr) {
        *this << std::string("display\r\n");
    }

    *this << static_cast<int8_t>('c');
    *this << ID;
    *this << static_cast<int8_t>(data);
}

void DSDisplay::addData(std::string ID, bool data) {
    // If packet is empty, add "display\r\n" header to packet
    if (sf::Packet::getData() == nullptr) {
        *this << std::string("display\r\n");
    }

    *this << static_cast<int8_t>('c');
    *this << ID;

    if (data == true) {
        *this << static_cast<int8_t>(DSDisplay::active);
    }
    else {
        *this <<
            static_cast<int8_t>(DSDisplay::inactive);
    }
}

void DSDisplay::addData(std::string ID, int8_t data) {
    // If packet is empty, add "display\r\n" header to packet
    if (sf::Packet::getData() == nullptr) {
        *this << std::string("display\r\n");
    }

    *this << static_cast<int8_t>('c');
    *this << ID;
    *this << static_cast<int8_t>(data);
}

void DSDisplay::addData(std::string ID, int32_t data) {
    // If packet is empty, add "display\r\n" header to packet
    if (sf::Packet::getData() == nullptr) {
        *this << std::string("display\r\n");
    }

    *this << static_cast<int8_t>('i');
    *this << ID;
    *this << static_cast<int32_t>(data);
}

void DSDisplay::addData(std::string ID, uint32_t data) {
    // If packet is empty, add "display\r\n" header to packet
    if (sf::Packet::getData() == nullptr) {
        *this << std::string("display\r\n");
    }

    *this << static_cast<int8_t>('u');
    *this << ID;
    *this << static_cast<uint32_t>(data);
}

void DSDisplay::addData(std::string ID, std::string data) {
    // If packet is empty, add "display\r\n" header to packet
    if (sf::Packet::getData() == nullptr) {
        *this << std::string("display\r\n");
    }

    *this << static_cast<int8_t>('s');
    *this << ID;
    *this << data;
}

void DSDisplay::addData(std::string ID, float data) {
    // If packet is empty, add "display\r\n" header to packet
    if (sf::Packet::getData() == nullptr) {
        *this << std::string("display\r\n");
    }

    *this << static_cast<int8_t>('s');
    *this << ID;

    std::stringstream ss;
    ss << data;
    *this << ss.str();
}

void DSDisplay::addData(std::string ID, double data) {
    // If packet is empty, add "display\r\n" header to packet
    if (sf::Packet::getData() == nullptr) {
        *this << std::string("display\r\n");
    }

    *this << static_cast<int8_t>('s');
    *this << ID;

    std::stringstream ss;
    ss << data;
    *this << ss.str();
}

