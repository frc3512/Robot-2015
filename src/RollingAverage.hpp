// =============================================================================
// File Name: RollingAverage.hpp
// Description: Creates queue of values and returns the average of the latest
//             values added to it
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#ifndef ROLLING_AVERAGE_HPP
#define ROLLING_AVERAGE_HPP

#include <pthread.h>
#include <atomic>

template <class T>
class RollingAverage {
public:
    RollingAverage(unsigned int size);
    virtual ~RollingAverage();

    void addValue(T value);
    void setSize(unsigned int newSize);
    T getAverage();

private:
    // Holds values to be averaged
    std::atomic<std::atomic<T>*> m_values;

    // Determines oldest value in array
    std::atomic<unsigned int> m_index;

    // Number of values to average
    std::atomic<unsigned int> m_size;

    // Total number of slots available in array (may be larger than m_size)
    std::atomic<unsigned int> m_maxSize;

    // Prevents accessing list from two places at once
    pthread_mutex_t m_dataMutex;
};

#include "RollingAverage.inl"

#endif // ROLLING_AVERAGE_HPP

