#include "Poller.h"
#include "Channel.h"
#include <cassert>
#include <iostream>

Poller::Poller(EventLoop* loop)
    : owner_loop(loop)
{
}

TimeStamp Poller::Poll(int timeout_ms, ChannelList& active_channels)
{
    int nums = ::poll(poll_fds.data(), poll_fds.size(), timeout_ms);
    auto now = TimeStamp::Now();
    if (nums > 0)
    {
        std::cout << nums << " events happended\n";
        FillActiveChannels(nums, active_channels);
    }
    else if (nums == 0)
    {
        std::cout << "nothing happended\n";
    }
    else
    {
        std::cout << "error in Poller::Poll()\n";
    }

    return now;
}

void Poller::FillActiveChannels(int num_events, ChannelList& active_channels) const
{
    for (const auto& poll_fd : poll_fds)
    {
        if (num_events <= 0)
            break;

        if (poll_fd.revents > 0)
        {
            --num_events;
            auto ch = channels.find(poll_fd.fd);
            auto channel = ch->second;
            channel->SetREvents(poll_fd.revents);

            active_channels.push_back(channel);
        }
    }
}

void Poller::UpdateChannel(Channel* channel)
{
    AssertInLoopThread();
    std::cout << "fd=" << channel->Fd() << ", event=" << channel->Events() << std::endl;

    if (channel->Index() < 0)
    {
        struct pollfd pfd;
        pfd.fd = channel->Fd();
        pfd.events = channel->Events();
        pfd.revents = 0;

        poll_fds.push_back(pfd);

        channel->SetIndex(poll_fds.size() - 1);
        channels.insert(std::make_pair(pfd.fd, channel));
    }
    else
    {
        auto& pfd = poll_fds.at(channel->Index());
        pfd.fd = channel->IsNoneEvent() ? -1 : pfd.fd;
        pfd.events = channel->Events();
        pfd.revents = 0;
    }
}
