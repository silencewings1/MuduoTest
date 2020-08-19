#include "EventLoop.h"
#include <unistd.h>
#include <iostream>

int main()
{
    EventLoop loop;
    std::cout << "main: pid = " << ::getpid() << ", tid = " << Tid() << std::endl;

    std::thread t([]() {
        std::cout << "thread: pid = " << ::getpid() << ", tid = " << Tid() << std::endl;

        EventLoop loop;
        loop.Loop();
    });

    t.join();

    loop.Loop();

    return 0;
}