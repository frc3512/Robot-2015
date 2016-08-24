// Copyright (c) FRC Team 3512, Spartatroniks 2015-2016. All Rights Reserved.

#include <unistd.h>

template <class T>
void SocketConnection::queueWrite(T& buf) {
    m_writequeue.emplace(reinterpret_cast<const char*>(&buf), sizeof(T));

    // Select on write
    selectflags |= SocketConnection::Write;
    write(m_ipcfd_w, "r", 1);
}
