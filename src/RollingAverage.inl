// =============================================================================
// File Name: RollingAverage.inl
// Description: Creates queue of values and returns the average of the latest
//             values added to it
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

template <class T>
RollingAverage<T>::RollingAverage(unsigned int size) :
    m_size(0),
    m_maxSize(size) {
    m_values = new std::atomic<T>[m_maxSize];
    pthread_mutex_init(&m_dataMutex, NULL);
}

template <class T>
RollingAverage<T>::~RollingAverage() {
    delete[] m_values;
    pthread_mutex_destroy(&m_dataMutex);
}

template <class T>
void RollingAverage<T>::addValue(T value) {
    /* Advance to next slot. If the next slot is past the end of the array, set
     * index to the beginning
     */
    m_index = (m_index + 1) % m_maxSize;

    /* Other assignments are atomic and don't negatively affect getAverage() */

    // Set oldest value to new value
    m_values[m_index] = value;

    if (m_size < m_maxSize) {
        m_size++;
    }
}

template <class T>
void RollingAverage<T>::setSize(unsigned int newSize) {
    std::atomic<T>* tempVals = new std::atomic<T>[newSize];

    unsigned int oldPos = m_index;
    unsigned int count = 0;
    while (count < newSize) {
        if (oldPos < 0) {
            oldPos = m_size - 1;
        }

        tempVals[count] = m_values[oldPos].load();

        oldPos--;
        count++;
    }

    m_maxSize = newSize;
    m_size = count;
    m_index = 0;

    // Swap buffers, then delete the old one
    m_values.exchange(tempVals);
    delete[] tempVals;
}

template <class T>
T RollingAverage<T>::getAverage() {
    T sum = 0;

    // Store values from atomic variables
    unsigned int index = m_index;
    unsigned int size = m_size;
    unsigned int maxSize = m_maxSize;

    for (unsigned int count = 0; count < size; count++) {
        sum += m_values[(index + count) % maxSize];
    }

    // Prevent divide by zero
    if (size != 0) {
        return sum / size;
    }
    else {
        return 0.f;
    }
}

