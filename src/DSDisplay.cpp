// =============================================================================
// File Name: DSDisplay.cpp
// Description: Receives IP address from remote host then sends HUD data there
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "DSDisplay.hpp"
#include <functional>
#include <fstream>
#include <cstring>
#include <iostream>

DSDisplay& DSDisplay::getInstance(unsigned short dsPort) {
    static DSDisplay dsDisplay(dsPort);
    return dsDisplay;
}

void DSDisplay::clear() {
    m_packet.clear();
}

void DSDisplay::sendToDS() {
    if (m_dsIP != sf::IpAddress::None) {
        m_socket.send(m_packet, m_dsIP, m_dsPort);
    }

    // Used for testing purposes
    m_socket.send(m_packet, sf::IpAddress(10, 35, 12, 42), m_dsPort);
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

            m_packet << static_cast<std::string>("guiCreate\r\n");

            // Open the file
            std::ifstream guiFile("/home/lvuser/GUISettings.txt",
                                  std::ifstream::binary);

            if (guiFile.is_open()) {
                // Get its length
                guiFile.seekg(0, guiFile.end);
                unsigned int fileSize = guiFile.tellg();
                guiFile.seekg(0, guiFile.beg);

                // Send the length
                m_packet << static_cast<uint32_t>(fileSize);

                // Allocate a buffer for the file
                char* tempBuf = new char[fileSize];

                // Send the data
                guiFile.read(tempBuf, fileSize);
                m_packet.append(tempBuf, fileSize);

                delete[] tempBuf;
                guiFile.close();
            }

            sendToDS();

            // Send a list of available autonomous modes
            clear();

            m_packet << static_cast<std::string>("autonList\r\n");

            for (unsigned int i = 0; i < m_autonModes.size(); i++) {
                m_packet << m_autonModes.name(i);
            }

            sendToDS();

            // Make sure driver knows which autonomous mode is selected
            clear();

            m_packet << static_cast<std::string>("autonConfirmed\r\n");
            m_packet << m_autonModes.name(m_curAutonMode);

            sendToDS();

            return "connect\r\n";
        }
        else if (std::strncmp(m_recvBuffer, "autonSelect\r\n", 13) == 0) {
            // Next byte after command is selection choice
            m_curAutonMode = m_recvBuffer[13];

            clear();

            m_packet << static_cast<std::string>("autonConfirmed\r\n");
            m_packet << m_autonModes.name(m_curAutonMode);

            // Store newest autonomous choice to file for persistent storage
            FILE* autonModeFile = fopen("/home/lvuser/autonMode.txt", "w");
            if (autonModeFile) {
                char temp[] = "auto";
                fwrite(temp, 1, 4, autonModeFile);
                fwrite(&m_curAutonMode, 1, sizeof(m_curAutonMode),
                       autonModeFile);

                fclose(autonModeFile);
            }
            else {
                std::cout <<
                    "DSDisplay: autonSelect: failed to open autonMode.txt\n";
            }

            sendToDS();

            return "autonSelect\r\n";
        }
    }

    return "NONE";
}

DSDisplay::DSDisplay(unsigned short portNumber) : m_dsPort(portNumber) {
    m_socket.bind(portNumber);
    m_socket.setBlocking(false);

    // Retrieve stored autonomous index
#if 0
    std::ifstream autonModeFile("/home/lvuser/autonMode.txt",
                                std::fstream::trunc);
    if (autonModeFile.good()) {
        char temp[4];
        autonModeFile.read(temp, 4);

        autonModeFile >> m_curAutonMode;
    }
    else {
        m_curAutonMode = 0;
    }
#endif

    FILE* autonModeFile = fopen("/home/lvuser/autonMode.txt", "r");
    if (autonModeFile) {
        char temp[4];
        fread(temp, 1, 4, autonModeFile);

        if (std::strcmp(temp, "auto") == 0) {
            fread(&m_curAutonMode, 1, sizeof(m_curAutonMode), autonModeFile);
        }
        else {
            m_curAutonMode = 0;
        }

        fclose(autonModeFile);
    }
    else {
        m_curAutonMode = 0;
    }
}

void DSDisplay::deleteAllMethods() {
    m_autonModes.deleteAllMethods();
}

void DSDisplay::execAutonomous() {
    m_autonModes.execAutonomous(m_curAutonMode);
}

char DSDisplay::getAutonID() const {
    return m_curAutonMode;
}

void DSDisplay::addData(std::string ID, StatusLight data) {
    // If packet is empty, add "display\r\n" header to packet
    if (m_packet.getData() == nullptr) {
        m_packet << std::string("display\r\n");
    }

    m_packet << static_cast<int8_t>('c');
    m_packet << ID;
    m_packet << static_cast<int8_t>(data);
}

void DSDisplay::addData(std::string ID, bool data) {
    // If packet is empty, add "display\r\n" header to packet
    if (m_packet.getData() == nullptr) {
        m_packet << std::string("display\r\n");
    }

    m_packet << static_cast<int8_t>('c');
    m_packet << ID;

    if (data == true) {
        m_packet << static_cast<int8_t>(DSDisplay::active);
    }
    else {
        m_packet << static_cast<int8_t>(DSDisplay::inactive);
    }
}

void DSDisplay::addData(std::string ID, int8_t data) {
    // If packet is empty, add "display\r\n" header to packet
    if (m_packet.getData() == nullptr) {
        m_packet << std::string("display\r\n");
    }

    m_packet << static_cast<int8_t>('c');
    m_packet << ID;
    m_packet << data;
}

void DSDisplay::addData(std::string ID, int32_t data) {
    // If packet is empty, add "display\r\n" header to packet
    if (m_packet.getData() == nullptr) {
        m_packet << std::string("display\r\n");
    }

    m_packet << static_cast<int8_t>('i');
    m_packet << ID;
    m_packet << data;
}

void DSDisplay::addData(std::string ID, uint32_t data) {
    // If packet is empty, add "display\r\n" header to packet
    if (m_packet.getData() == nullptr) {
        m_packet << std::string("display\r\n");
    }

    m_packet << static_cast<int8_t>('u');
    m_packet << ID;
    m_packet << data;
}

void DSDisplay::addData(std::string ID, std::string data) {
    // If packet is empty, add "display\r\n" header to packet
    if (m_packet.getData() == nullptr) {
        m_packet << std::string("display\r\n");
    }

    m_packet << static_cast<int8_t>('s');
    m_packet << ID;
    m_packet << data;
}

void DSDisplay::addData(std::string ID, float data) {
    // If packet is empty, add "display\r\n" header to packet
    if (m_packet.getData() == nullptr) {
        m_packet << std::string("display\r\n");
    }

    m_packet << static_cast<int8_t>('s');
    m_packet << ID;
    m_packet << std::to_string(data);
}

void DSDisplay::addData(std::string ID, double data) {
    // If packet is empty, add "display\r\n" header to packet
    if (m_packet.getData() == nullptr) {
        m_packet << std::string("display\r\n");
    }

    m_packet << static_cast<int8_t>('s');
    m_packet << ID;
    m_packet << std::to_string(data);
}

