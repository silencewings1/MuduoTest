#include "Channel.h"
#include "EventLoop.h"
#include <iostream>
#include <string.h>
#include <sys/timerfd.h>
#include <unistd.h>

int main()
{
    EventLoop loop;

    int timer_fd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    struct itimerspec howlong;
    bzero(&howlong, sizeof(howlong));
    howlong.it_value.tv_sec = 2;
    ::timerfd_settime(timer_fd, 0, &howlong, nullptr);

    Channel channel(&loop, timer_fd);
    channel.SetReadCallback([&loop]() {
        std::cout << "Timeout!\n";

        thread_local int count = 3;
        if (--count <= 0)
            loop.Quit();
    });
    channel.EnableReading();

    loop.Loop();

    ::close(timer_fd);
    return 0;
}