#include "Channel.h"
#include "EventLoop.h"
#include <iostream>
#include <poll.h>

const int Channel::none_event = 0;
const int Channel::read_event = POLLIN | POLLPRI;
const int Channel::write_event = POLLOUT;

Channel::Channel(EventLoop* loop, int fd)
    : loop(loop)
    , fd(fd)
    , events(0)
    , revents(0)
    , index(-1)
{
}

void Channel::Update()
{
    loop->UpdateChannel(this);
}

void Channel::HandleEvent()
{
    if (revents & POLLNVAL)
        std::cout << "Channel::handle_event() POLLNVAL" << std::endl;

    if (revents & (POLLERR | POLLNVAL))
        if (error_cb)
            error_cb();

    if (revents & (POLLIN | POLLPRI | POLLRDHUP))
        if (read_cb)
            read_cb();

    if (revents & POLLOUT)
        if (write_cb)
            write_cb();
}