#include <unistd.h>

/* Queue a buffer for writing. Returns 0 on success, returns -1 if buffer
 * wasn't queued. Only one buffer can be queued for writing at a time.
 */
template <class T>
int SocketConnection::queuewrite(T& buf) {
    m_writequeue.emplace(reinterpret_cast<const char*>(&buf), sizeof(T));

    // Select on write
    selectflags |= SocketConnection::Write;
    write(m_ipcfd_w, "r", 1);

    return 0;
}

