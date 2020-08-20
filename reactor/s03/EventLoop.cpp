#include "EventLoop.h"
#include "Channel.h"
#include "Poller.h"
#include "TimerQueue.h"
#include <cassert>
#include <iostream>

thread_local EventLoop* loop_in_this_thread = nullptr;
constexpr int POLL_TIME_MS = 10000;

EventLoop::EventLoop()
    : looping(false)
    , quit(false)
    , thread_id(Tid())
    , poller(std::make_unique<Poller>(this))
    , timer_queue(std::make_unique<TimerQueue>(this))
    , poll_return_time(TimeStamp::Invaild())
{
    std::cout << "EventLoop Create " << this
              << " in thread " << thread_id << std::endl;
    if (loop_in_this_thread)
    {
        std::cout << "Another EventLoop " << loop_in_this_thread
                  << " exists in this thread " << thread_id << std::endl;
        abort();
    }
    else
    {
        loop_in_this_thread = this;
    }
}

EventLoop::~EventLoop()
{
    loop_in_this_thread = nullptr;
}

void EventLoop::Loop()
{
    AssertInLoopThread();
    looping = true;
    quit = false;

    Poller::ChannelList active_channels;
    while (!quit)
    {
        active_channels.clear();

        poll_return_time = poller->Poll(POLL_TIME_MS, active_channels);

        for (auto& ch : active_channels)
        {
            ch->HandleEvent();
        }
    }
}

void EventLoop::Quit()
{
    quit = true;
}

TimerId EventLoop::RunAt(const TimeStamp& when, TimerCallback cb)
{
    return timer_queue->AddTimer(std::move(cb), when);
}

TimerId EventLoop::RunAfter(const Duration& delay, TimerCallback cb)
{
    return timer_queue->AddTimer(std::move(cb),
                                 AddTime(TimeStamp::Now(), delay));
}

TimerId EventLoop::RunEvery(const Duration& interval, TimerCallback cb)
{
    return timer_queue->AddTimer(std::move(cb),
                                 AddTime(TimeStamp::Now(), interval),
                                 interval);
}

void EventLoop::UpdateChannel(Channel* channel)
{
    AssertInLoopThread();
    poller->UpdateChannel(channel);
}

void EventLoop::AssertInLoopThread()
{
    if (!IsInLoopThread())
    {
        AbortNotInLoopThread();
    }
}

bool EventLoop::IsInLoopThread() const
{
    return thread_id == Tid();
}

void EventLoop::AbortNotInLoopThread()
{
    std::cout << "EventLoop::abortNotInLoopThread - EventLoop " << this
              << " was created in threadId_ = " << thread_id
              << ", current thread id = " << Tid() << std::endl;
    abort();
}