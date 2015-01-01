//=============================================================================
//File Name: DriverStationDisplay.cpp
//Description: Receives IP address from remote host then sends HUD data there
//Author: FRC Team 3512, Spartatroniks
//=============================================================================

#include "DriverStationDisplay.hpp"

sf::UdpSocket* SocketInit::m_socket;

SocketInit::SocketInit() {
}

SocketInit::~SocketInit() {
    m_socket->unbind();
    delete m_socket;
}

sf::UdpSocket& SocketInit::getInstance( unsigned short portNumber ) {
    if ( m_socket == NULL ) {
        m_socket = new sf::UdpSocket;
        if ( portNumber == 0 ) {
            m_socket->bind( 1130 );
        }
        else {
            m_socket->bind( portNumber );
        }
        m_socket->setBlocking( false );
    }

    return *m_socket;
}

SocketInit gSocket;
