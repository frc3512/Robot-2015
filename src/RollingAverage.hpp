// =============================================================================
// File Name: RollingAverage.hpp
// Description: Creates queue of values and returns the average of the latest
//              values added to it
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#ifndef ROLLING_AVERAGE_HPP
#define ROLLING_AVERAGE_HPP

#include <atomic>

// T is type of value to be averaged; N is total number of values to average
template <class T, size_t N>
class RollingAverage {
public:
    void addValue(T value);

    // Returns current average
    T get();

private:
    std::atomic<T> m_values[N];

    // Holds current sum
    std::atomic<T> m_sum{0};

    // Determines oldest value in array
    std::atomic<unsigned int> m_index{0};

    // Number of values to average (may be less than maximum N)
    std::atomic<unsigned int> m_size{0};
};

#include "RollingAverage.inl"

#endif // ROLLING_AVERAGE_HPP

