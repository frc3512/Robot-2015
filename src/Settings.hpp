// Copyright (c) FRC Team 3512, Spartatroniks 2015-2016. All Rights Reserved.

#pragma once

#include <map>
#include <string>

/**
 * Opens a given file and creates an STL map of its name-value pairs
 */
class Settings {
public:
    explicit Settings(std::string fileName);

    // Updates list of values from given file
    void Update();

    /* Returns value associated with the given key
     * Returns "NOT_FOUND" if there is no entry for that name-value pair
     */
    std::string GetString(const std::string& key) const;

    /* Returns value associated with the given key
     * Returns 0 if there is no entry for that name-value pair
     */
    double GetDouble(const std::string& key) const;

    /* Returns value associated with the given key
     * Returns 0 if there is no entry for that name-value pair
     */
    int GetInt(const std::string& key) const;

    // Saves all name-value pairs to external file with the given name
    void SaveToFile(const std::string& fileName);

private:
    std::string m_fileName;
    std::map<std::string, std::string> m_values;

    // Used when processing strings from external file
    std::string m_rawStr;

    // Used when stepping through m_rawStr
    size_t m_index = 0;

    std::string ExtractKey();
    std::string ExtractValue();
};
