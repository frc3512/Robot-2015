// =============================================================================
// File Name: RollingAverage.inl
// Description: Creates queue of values and returns the average of the latest
//              values added to it
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

template <class T, size_t N>
void RollingAverage<T, N>::addValue(T value) {
    /* Advance to next slot. If the next slot is past the end of the array, set
     * index to the beginning
     */
    m_index++;
    if (m_index == N) {
        m_index = 0;
    }

    // Remove old value before adding new one
    if (m_size == N) {
        m_sum = m_sum - m_values[m_index];
    }
    else {
        m_size++;
    }

    // Set oldest value to new value
    m_values[m_index] = value;

    // Add new value to rolling sum
    m_sum = m_sum + value;
}

template <class T, size_t N>
T RollingAverage<T, N>::get() {
    // Prevent divide by zero
    if (m_size != 0) {
        return m_sum / m_size;
    }
    else {
        return 0.f;
    }
}

