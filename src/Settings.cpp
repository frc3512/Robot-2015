//=============================================================================
//File Name: Settings.cpp
//Description: Opens a given file and creates an STL map of its name-value
//             pairs
//Author: FRC Team 3512, Spartatroniks
//=============================================================================

#include "Settings.hpp"
#include <fstream>
#include <iostream>
#include <cstdlib>

Settings::Settings( std::string fileName ) :
        m_fileName( fileName ) ,
        m_index( 0 ) {
    update();
}

Settings::~Settings() {
}

void Settings::update() {
    std::string name;
    std::string value;

    m_values.clear();

    std::ifstream settings( m_fileName.c_str() );
    if ( !settings.is_open() ) {
        std::cout << "Failed to open " << m_fileName << "\n";
        return;
    }

    do {
        std::getline( settings , m_rawStr );

        m_index = 0;
        name = extractDataFromString( true );
        value = extractDataFromString( false );

        // Add name-value pair to map
        m_values[name] = value;
    } while ( !settings.eof() );

    settings.close();

    std::cout << "Settings loaded from " << m_fileName << "\n";
}

const std::string Settings::getString( const std::string& key ) const {
    std::map<std::string , std::string>::const_iterator const index = m_values.find( key );

    // If the element wasn't found
    if ( index == m_values.end() ) {
        std::cout << "Settings Error: '" << key << "' not found\n";
        return "NOT_FOUND";
    }

    // Else return the value for that element
    return index->second;
}

const float Settings::getFloat( const std::string& key ) const {
    std::map<std::string , std::string>::const_iterator index = m_values.find( key );

    // If the element wasn't found
    if ( index == m_values.end() ) {
        std::cout << "Settings Error: '" << key << "' not found\n";
        return 0.f;
    }

    // Else return the value for that element
    return atof( index->second.c_str() );
}

const int Settings::getInt( const std::string& key ) const {
    std::map<std::string , std::string>::const_iterator index = m_values.find( key );

    // If the element wasn't found
    if ( index == m_values.end() ) {
        std::cout << "Settings Error: '" << key << "' not found\n";
        return 0;
    }

    // Else return the value for that element
    return atoi( index->second.c_str() );

}

void Settings::saveToFile( const std::string& fileName ) {
    std::ofstream outFile( fileName.c_str() , std::ios_base::out | std::ios_base::trunc );
    if ( outFile.is_open() ) {
        for ( std::map<std::string , std::string>::iterator index = m_values.begin() ; index != m_values.end() ; index++ ) {
            outFile << index->first << " = " << index->second << "\n";
        }

        outFile.close();
    }
}

std::string Settings::extractDataFromString( const bool& isName ) {
    std::string value;
    bool hasEquals = false;

    // Find start of name
    while ( ( m_rawStr[m_index] == ' ' || m_rawStr[m_index] == '\t' ) && m_index < m_rawStr.length() ) {
        m_index++;
    }
    if ( m_index == m_rawStr.length() ) {
        return value;
    }

    size_t valueStart = m_index;

    // Find end of name
    while ( m_rawStr[m_index] != ' ' && m_rawStr[m_index] != '\t' && m_rawStr[m_index] != '=' && m_index < m_rawStr.length() ) {
        m_index++;
    }
    if ( m_index == m_rawStr.length() && isName ) {
        return value;
    }

    value = m_rawStr.substr( valueStart , m_index - valueStart );

    if ( isName ) {
        // If end of name was delimited by an equals sign
        if ( m_rawStr[m_index] == '=' ) {
            hasEquals = true;
        }

        // Find an equals sign if there wasn't already one
        while ( !hasEquals && m_rawStr[m_index] != '=' && m_index < m_rawStr.length() ) {
            m_index++;
        }
        if ( m_index == m_rawStr.length() ) {
            return value;
        }

        /* Since the last character was an equals sign in either case,
         * advance to the next character for finding the value
         */
        m_index++;
    }

    return value;
}
