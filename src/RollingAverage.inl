// =============================================================================
// File Name: RollingAverage.inl
// Description: Creates queue of values and returns the average of the latest
//              values added to it
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

template <class T, size_t N>
RollingAverage<T, N>::RollingAverage() :
    m_size(0),
    m_maxSize(N) {
}

template <class T, size_t N>
void RollingAverage<T, N>::addValue(T value) {
    // Remove old value before adding new one
    if (m_size > 0 && m_size < m_maxSize) {
        m_sum = m_sum - value;
    }

    /* Advance to next slot. If the next slot is past the end of the array, set
     * index to the beginning
     */
    m_index = (m_index + 1) % m_maxSize;

    // Other assignments are atomic and don't negatively affect getAverage()

    // Set oldest value to new value
    m_values[m_index] = value;
    m_sum = m_sum + value;

    if (m_size < m_maxSize) {
        m_size++;
    }
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

