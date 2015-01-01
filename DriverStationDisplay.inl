//=============================================================================
//File Name: DriverStationDisplay.inl
//Description: Receives IP address from remote host then sends HUD data there
//Author: FRC Team 3512, Spartatroniks
//=============================================================================

template <class T>
DriverStationDisplay<T>* DriverStationDisplay<T>::m_dsDisplay = NULL;

template <class T>
DriverStationDisplayInit<T>::DriverStationDisplayInit() {
}

template <class T>
DriverStationDisplayInit<T>::~DriverStationDisplayInit() {
    delete DriverStationDisplay<T>::m_dsDisplay;
}

template <class T>
DriverStationDisplay<T>::~DriverStationDisplay() {
}

template <class T>
DriverStationDisplay<T>* DriverStationDisplay<T>::getInstance( unsigned short dsPort ) {
    if ( m_dsDisplay == NULL ) {
        m_dsDisplay = new DriverStationDisplay<T>( dsPort );
    }

    return m_dsDisplay;
}

template <class T>
void DriverStationDisplay<T>::clear() {
    static_cast<sf::Packet*>(this)->clear();
}

template <class T>
void DriverStationDisplay<T>::sendToDS( sf::Packet* userData ) {
    if ( m_dsIP != sf::IpAddress::None ) {
        if ( userData == NULL ) {
            SocketInit::getInstance().send( *static_cast<sf::Packet*>(this) , m_dsIP , m_dsPort );
        }
        else {
            SocketInit::getInstance().send( *userData , m_dsIP , m_dsPort );
        }
    }

    // Used for testing purposes
    sf::IpAddress testIP( 10 , 35 , 12 , 42 );
    if ( userData == NULL ) {
        SocketInit::getInstance().send( *static_cast<sf::Packet*>(this) , testIP , m_dsPort );
    }
    else {
        SocketInit::getInstance().send( *userData , testIP , m_dsPort );
    }
}

template <class T>
const std::string DriverStationDisplay<T>::receiveFromDS() {
    if ( SocketInit::getInstance().receive( m_recvBuffer , 256 , m_recvAmount , m_recvIP , m_recvPort ) == sf::Socket::Done ) {
        if ( std::strncmp( m_recvBuffer , "connect\r\n" , 9 ) == 0 ) {
            m_dsIP = m_recvIP;
            m_dsPort = m_recvPort;

            // Send GUI element file to DS
            clear();

            *this << static_cast<std::string>( "guiCreate\r\n" );

            // Open the file
            // FIXME: May crash if file has \n for newlines instead of \r\n
            std::ifstream guiFile(
                    "GUISettings.txt" , std::ifstream::binary );

            if ( guiFile.is_open() ) {
                // Get its length
                guiFile.seekg( 0 , guiFile.end );
                unsigned int fileSize = guiFile.tellg();
                guiFile.seekg( 0 , guiFile.beg );

                // Send the length
                *this << static_cast<uint32_t>(fileSize);

                // Allocate a buffer for the file
                char* tempBuf = new char[fileSize];

                // Send the data TODO: htonl() the data before it's sent
                guiFile.read( tempBuf , fileSize );
                append( tempBuf , fileSize );

                delete[] tempBuf;
                guiFile.close();
            }

            sendToDS();

            // Send a list of available autonomous modes
            clear();

            *this << static_cast<std::string>( "autonList\r\n" );

            for ( unsigned int i = 0 ; i < m_autonModes.size() ; i++ ) {
                *this << m_autonModes.name( i );
            }

            sendToDS();

            // Make sure driver knows which autonomous mode is selected
            clear();

            *this << static_cast<std::string>( "autonConfirmed\r\n" );
            *this << m_autonModes.name( curAutonMode );

            sendToDS();

            return "connect\r\n";
        }
        else if ( std::strncmp( m_recvBuffer , "autonSelect\r\n" , 13 ) == 0 ) {
            // Next byte after command is selection choice
            curAutonMode = m_recvBuffer[13];

            clear();

            *this << static_cast<std::string>( "autonConfirmed\r\n" );
            *this << m_autonModes.name( curAutonMode );

            // Store newest autonomous choice to file for persistent storage
            std::ofstream autonModeFile( "autonMode.txt" , std::ofstream::trunc );
            if ( autonModeFile.is_open() ) {
                autonModeFile << curAutonMode;

                autonModeFile.close();
            }

            sendToDS();

            return "autonSelect\r\n";
        }
    }

    return "NONE";
}

template <class T>
DriverStationDisplay<T>::DriverStationDisplay( unsigned short portNumber ) : m_dsIP( sf::IpAddress::None ) , m_dsPort( portNumber ) {
    m_recvIP = sf::IpAddress( 0 , 0 , 0 , 0 );
    m_recvPort = 0;
    m_recvAmount = 0;

    SocketInit::getInstance( portNumber );

    // Retrieve stored autonomous index
    std::ifstream autonModeFile( "autonMode.txt" );
    if ( autonModeFile.is_open() ) {
        autonModeFile >> curAutonMode;

        autonModeFile.close();
    }
    else {
        curAutonMode = 0;
    }
}

template <class T>
void DriverStationDisplay<T>::addAutonMethod( const std::string& methodName , void (T::*function)() , T* object ) {
    m_autonModes.addMethod( methodName , function , object );
}

template <class T>
void DriverStationDisplay<T>::deleteAllMethods() {
    m_autonModes.deleteAllMethods();
}

template <class T>
void DriverStationDisplay<T>::execAutonomous() {
    m_autonModes.execAutonomous( curAutonMode );
}

namespace DS {
template <class T>
inline void AddElementData( DriverStationDisplay<T>* inst , std::string ID , DS::StatusLight data ) {
    // If packet is empty, add "display\r\n" header to packet
    if ( static_cast<sf::Packet*>(inst)->getData() == NULL ) {
        *static_cast<sf::Packet*>(inst) << std::string( "display\r\n" );
    }

    *static_cast<sf::Packet*>(inst) << static_cast<int8_t>('c');
    *static_cast<sf::Packet*>(inst) << ID;
    *static_cast<sf::Packet*>(inst) << static_cast<int8_t>(data);
}

template <class T>
inline void AddElementData( DriverStationDisplay<T>* inst , std::string ID , bool data ) {
    // If packet is empty, add "display\r\n" header to packet
    if ( static_cast<sf::Packet*>(inst)->getData() == NULL ) {
        *static_cast<sf::Packet*>(inst) << std::string( "display\r\n" );
    }

    *static_cast<sf::Packet*>(inst) << static_cast<int8_t>('c');
    *static_cast<sf::Packet*>(inst) << ID;

    if ( data == true ) {
        *static_cast<sf::Packet*>(inst) << static_cast<int8_t>(DS::active);
    }
    else {
        *static_cast<sf::Packet*>(inst) << static_cast<int8_t>(DS::inactive);
    }
}

template <class T>
inline void AddElementData( DriverStationDisplay<T>* inst , std::string ID , int8_t data ) {
    // If packet is empty, add "display\r\n" header to packet
    if ( static_cast<sf::Packet*>(inst)->getData() == NULL ) {
        *static_cast<sf::Packet*>(inst) << std::string( "display\r\n" );
    }

    *static_cast<sf::Packet*>(inst) << static_cast<int8_t>('c');
    *static_cast<sf::Packet*>(inst) << ID;
    *static_cast<sf::Packet*>(inst) << static_cast<int8_t>(data);
}

template <class T>
inline void AddElementData( DriverStationDisplay<T>* inst , std::string ID , int32_t data ) {
    // If packet is empty, add "display\r\n" header to packet
    if ( static_cast<sf::Packet*>(inst)->getData() == NULL ) {
        *static_cast<sf::Packet*>(inst) << std::string( "display\r\n" );
    }

    *static_cast<sf::Packet*>(inst) << static_cast<int8_t>('i');
    *static_cast<sf::Packet*>(inst) << ID;
    *static_cast<sf::Packet*>(inst) << static_cast<int32_t>(data);
}

template <class T>
inline void AddElementData( DriverStationDisplay<T>* inst , std::string ID , uint32_t data ) {
    // If packet is empty, add "display\r\n" header to packet
    if ( static_cast<sf::Packet*>(inst)->getData() == NULL ) {
        *static_cast<sf::Packet*>(inst) << std::string( "display\r\n" );
    }

    *static_cast<sf::Packet*>(inst) << static_cast<int8_t>('u');
    *static_cast<sf::Packet*>(inst) << ID;
    *static_cast<sf::Packet*>(inst) << static_cast<uint32_t>(data);
}

template <class T>
inline void AddElementData( DriverStationDisplay<T>* inst , std::string ID , std::string data ) {
    // If packet is empty, add "display\r\n" header to packet
    if ( static_cast<sf::Packet*>(inst)->getData() == NULL ) {
        *static_cast<sf::Packet*>(inst) << std::string( "display\r\n" );
    }

    *static_cast<sf::Packet*>(inst) << static_cast<int8_t>('s');
    *static_cast<sf::Packet*>(inst) << ID;
    *static_cast<sf::Packet*>(inst) << data;
}

template <class T>
inline void AddElementData( DriverStationDisplay<T>* inst , std::string ID , float data ) {
    // If packet is empty, add "display\r\n" header to packet
    if ( static_cast<sf::Packet*>(inst)->getData() == NULL ) {
        *static_cast<sf::Packet*>(inst) << std::string( "display\r\n" );
    }

    *static_cast<sf::Packet*>(inst) << static_cast<int8_t>('s');
    *static_cast<sf::Packet*>(inst) << ID;

    std::stringstream ss;
    ss << data;
    *static_cast<sf::Packet*>(inst) << ss.str();
}

template <class T>
inline void AddElementData( DriverStationDisplay<T>* inst , std::string ID , double data ) {
    // If packet is empty, add "display\r\n" header to packet
    if ( static_cast<sf::Packet*>(inst)->getData() == NULL ) {
        *static_cast<sf::Packet*>(inst) << std::string( "display\r\n" );
    }

    *static_cast<sf::Packet*>(inst) << static_cast<int8_t>('s');
    *static_cast<sf::Packet*>(inst) << ID;

    std::stringstream ss;
    ss << data;
    *static_cast<sf::Packet*>(inst) << ss.str();
}
}
