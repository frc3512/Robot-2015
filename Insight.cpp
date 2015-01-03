//=============================================================================
//File Name: Insight.cpp
//Description: Receives Insight's processed target data
//Author: FRC Team 3512, Spartatroniks
//=============================================================================

#include "Insight.hpp"

Insight* Insight::m_insight = nullptr;

InsightInit::InsightInit() {
}

InsightInit::~InsightInit() {
    delete Insight::m_insight;
}

Insight::~Insight() {
    m_socket.unbind();
}

Insight* Insight::getInstance( unsigned short dsPort ) {
    if ( m_insight == nullptr ) {
        m_insight = new Insight( dsPort );
    }

    return m_insight;
}

const std::string Insight::receiveFromDS() {
    if ( m_socket.receive( m_recvBuffer , 256 , m_recvAmount , m_recvIP , m_recvPort ) == sf::Socket::Done ) {
        if ( std::strncmp( m_recvBuffer , "ctrl\r\n" , 6 ) == 0 ) {
            m_targets.clear();

            for ( unsigned int i = 0 ; i < 3 ; i++ ) {
                if ( !(m_recvBuffer[8+i*2] == 0 && m_recvBuffer[9+i*2] == 0) ) {
                    m_targets.push_back( std::make_pair( m_recvBuffer[8+i*2] , m_recvBuffer[9+i*2] ) );
                }
            }

            m_hasNewData = true;

            return "ctrl\r\n";
        }
    }
    else {
        m_hasNewData = false;
    }

    return "NONE";
}

bool Insight::hasNewData() const {
    return m_hasNewData;
}

const std::pair<char,char>& Insight::getTarget( size_t i ) {
    return m_targets[i];
}

size_t Insight::getNumTargets() const {
    return m_targets.size();
}

Insight::Insight( unsigned short portNumber ) {
    m_socket.bind( portNumber );
    m_socket.setBlocking( false );
    m_recvIP = sf::IpAddress( 0 , 0 , 0 , 0 );
    m_recvPort = 0;
    m_recvAmount = 0;
    m_hasNewData = false;
}
