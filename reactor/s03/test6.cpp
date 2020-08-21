#include "EventLoop.h"
#include "EventLoopThread.h"
#include <chrono>
#include <iostream>
#include <unistd.h>
using namespace std::chrono_literals;

void Fun()
{
    std::cout << "Fun: pid=" << getpid() << ", tid=" << Tid() << std::endl;
}

int main()
{
    std::cout << "main: pid=" << getpid() << ", tid=" << Tid() << std::endl;

    {
        EventLoopThread loop_thread;
        auto loop = loop_thread.StartLoop();

        loop->RunInLoop(Fun);
        std::this_thread::sleep_for(1s);
        loop->RunAfter(2s, Fun);
        std::this_thread::sleep_for(3s);

        //getchar();
        loop->Quit();
    }

    std::cout << "exit main\n";

    return 0;
}