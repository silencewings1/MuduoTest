#include "EventLoop.h"
#include <unistd.h>

EventLoop* g_loop;

int main()
{
    g_loop = new EventLoop();

    std::thread t([]() {
        g_loop->Loop();
    });

    t.join();

    return 0;
}