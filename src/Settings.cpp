// =============================================================================
// File Name: Settings.cpp
// Description: Opens a given file and creates an STL map of its name-value
//              pairs
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include "Settings.hpp"
#include <fstream>
#include <iostream>

Settings::Settings(std::string fileName) : m_fileName(fileName) {
    update();
}

void Settings::update() {
    std::string name;
    std::string value;

    m_values.clear();

    std::ifstream settings(m_fileName.c_str());
    if (!settings.is_open()) {
        std::cout << "Failed to open " << m_fileName << "\n";
        return;
    }

    do {
        std::getline(settings, m_rawStr);

        m_index = 0;
        name = extractKey();
        value = extractValue();

        // Add name-value pair to map
        m_values[name] = value;
    } while (!settings.eof());

    settings.close();

    std::cout << "Settings loaded from " << m_fileName << "\n";
}

std::string Settings::getString(const std::string& key) const {
    auto index = m_values.find(key);

    // If the element wasn't found
    if (index == m_values.end()) {
        std::cout << "Settings: " << m_fileName << ": '" << key <<
            "' not found\n";
        return "NOT_FOUND";
    }

    // Else return the value for that element
    return index->second;
}

double Settings::getDouble(const std::string& key) const {
    auto index = m_values.find(key);

    // If the element wasn't found
    if (index == m_values.end()) {
        std::cout << "Settings: " << m_fileName << ": '" << key <<
            "' not found\n";
        return 0.f;
    }

    // Else return the value for that element
    return atof(index->second.c_str());
}

int Settings::getInt(const std::string& key) const {
    auto index = m_values.find(key);

    // If the element wasn't found
    if (index == m_values.end()) {
        std::cout << "Settings: " << m_fileName << ": '" << key <<
            "' not found\n";
        return 0;
    }

    // Else return the value for that element
    return atoi(index->second.c_str());
}

void Settings::saveToFile(const std::string& fileName) {
    std::ofstream outFile(fileName, std::ios_base::out | std::ios_base::trunc);

    if (outFile.is_open()) {
        for (auto index : m_values) {
            outFile << index.first << " = " << index.second << "\n";
        }

        outFile.close();
    }
}

std::string Settings::extractKey() {
    // Find start of name
    m_index = m_rawStr.find_first_not_of(" \t", m_index);
    if (m_index == std::string::npos) {
        return "";
    }

    size_t keyStart = m_index;

    // Find end of name
    m_index = m_rawStr.find_first_of(" \t=", m_index);

    return m_rawStr.substr(keyStart, m_index - keyStart);
}

std::string Settings::extractValue() {
    // Find start of value
    m_index = m_rawStr.find_first_not_of(" \t=", m_index);
    if (m_index == std::string::npos) {
        return "";
    }

    size_t valueStart = m_index;

    // Find end of value
    m_index = m_rawStr.find_first_of(" \t", m_index);

    return m_rawStr.substr(valueStart, m_index - valueStart);
}

