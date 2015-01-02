// A very simple test harness for GraphHost

#if 0
#include <chrono>
#include <cstdint>
#include <unistd.h>
#include <signal.h>
#include "graphhost.hpp"

int main() {
    graphhost_t gh( 4098 );

    // Ignore SIGPIPE
    signal( SIGPIPE , SIG_IGN );

    uint32_t startTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    uint32_t currentTime = startTime;

    // Send some bogus data
    while ( 1 ) {
        currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        gh.graphData(currentTime - startTime, 0, "PID0");
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    return 0;
}

#endif
