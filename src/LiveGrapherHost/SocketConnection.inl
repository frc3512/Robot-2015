// =============================================================================
// File Name: SocketConnection.inl
// Description: Wrapper around graph client socket descriptors
// Author: FRC Team 3512, Spartatroniks
// =============================================================================

#include <unistd.h>

template <class T>
void SocketConnection::queueWrite(T& buf) {
    m_writequeue.emplace(reinterpret_cast<const char*>(&buf), sizeof(T));

    // Select on write
    selectflags |= SocketConnection::Write;
    write(m_ipcfd_w, "r", 1);
}

