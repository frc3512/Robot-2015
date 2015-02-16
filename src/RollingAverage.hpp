// =============================================================================
// File Name: RollingAverage.hpp
// Description: Creates queue of values and returns the average of the latest
//              values added to it
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#ifndef ROLLING_AVERAGE_HPP
#define ROLLING_AVERAGE_HPP

#include <atomic>

template <class T, size_t N>
class RollingAverage {
public:
    RollingAverage();

    void addValue(T value);

    // Returns current average
    T get();

private:
    std::atomic<T> m_values[N];

    // Holds current sum
    std::atomic<T> m_sum;

    // Determines oldest value in array
    std::atomic<unsigned int> m_index;

    // Number of values to average
    std::atomic<unsigned int> m_size;

    // Total number of slots available in array (may be larger than m_size)
    const unsigned int m_maxSize = N;
};

#include "RollingAverage.inl"

#endif // ROLLING_AVERAGE_HPP

