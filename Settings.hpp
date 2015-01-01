//=============================================================================
//File Name: Settings.hpp
//Description: Opens a given file and creates an STL map of its name-value
//             pairs
//Author: FRC Team 3512, Spartatroniks
//=============================================================================

#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <map>
#include <string>

class Settings {
public:
    Settings( std::string fileName );
    virtual ~Settings();

    // Updates list of values from given file
    void update();

    /* Returns value associated with the given key
     * Returns "NOT_FOUND" if there is no entry for that name-value pair
     */
    const std::string getString( const std::string& key ) const;

    /* Returns value associated with the given key
     * Returns 0 if there is no entry for that name-value pair
     */
    const float getFloat( const std::string& key ) const;

    /* Returns value associated with the given key
     * Returns 0 if there is no entry for that name-value pair
     */
    const int getInt( const std::string& key ) const;

    // Saves all name-value pairs to external file with the given name
    void saveToFile( const std::string& fileName );

private:
    std::string m_fileName;
    std::map<std::string , std::string> m_values;

    // Used when processing strings from external file
    std::string m_rawStr;

    // Used when stepping through m_rawStr
    size_t m_index;

    std::string extractDataFromString( const bool& isName );
};

#endif // SETTINGS_HPP
