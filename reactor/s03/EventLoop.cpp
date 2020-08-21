#include "EventLoop.h"
#include "Channel.h"
#include "Poller.h"
#include "TimerQueue.h"
#include <cassert>
#include <iostream>
#include <sys/eventfd.h>
#include <unistd.h>

namespace
{

thread_local EventLoop* loop_in_this_thread = nullptr;
constexpr int POLL_TIME_MS = 10000;

int CreateEventFd()
{
    int fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (fd < 0)
    {
        std::cout << "Failed in eventfd\n";
        abort();
    }

    return fd;
}

} // namespace

EventLoop::EventLoop()
    : looping(false)
    , quit(false)
    , calling_pending_functors(false)
    , thread_id(Tid())
    , poller(std::make_unique<Poller>(this))
    , timer_queue(std::make_unique<TimerQueue>(this))
    , poll_return_time(TimeStamp::Invaild())
    , wakeup_fd(CreateEventFd())
    , wakeup_channel(std::make_unique<Channel>(this, wakeup_fd))
{
    // std::cout << "EventLoop Create " << this
    //           << " in thread " << thread_id << std::endl;
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

    wakeup_channel->SetReadCallback([this]() { this->HandleRead(); });
    wakeup_channel->EnableReading();
}

EventLoop::~EventLoop()
{
    loop_in_this_thread = nullptr;
    ::close(wakeup_fd);
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

        DoPendingFunctors();
    }

    looping = false;
}

void EventLoop::Quit()
{
    quit = true;

    if (!IsInLoopThread())
    {
        WakeUp();
    }
}

void EventLoop::RunInLoop(const Functor& cb)
{
    if (IsInLoopThread())
    {
        cb();
    }
    else
    {
        QueueInLoop(cb);
    }
}

void EventLoop::QueueInLoop(const Functor& cb)
{
    {
        std::lock_guard lock(mutex_);
        pending_functors.push_back(cb);
    }

    if (!IsInLoopThread() || calling_pending_functors)
    {
        WakeUp();
    }
}

TimerId EventLoop::RunAt(const TimeStamp& when, const TimerCallback& cb)
{
    return timer_queue->AddTimer(cb, when);
}

TimerId EventLoop::RunAfter(const Duration& delay, const TimerCallback& cb)
{
    return timer_queue->AddTimer(cb,
                                 AddTime(TimeStamp::Now(), delay));
}

TimerId EventLoop::RunEvery(const Duration& interval, const TimerCallback& cb)
{
    return timer_queue->AddTimer(cb,
                                 AddTime(TimeStamp::Now(), interval),
                                 interval);
}

void EventLoop::WakeUp()
{
    uint64_t one = 1;
    auto n = ::write(wakeup_fd, &one, sizeof(one));

    if (n != sizeof(one))
    {
        std::cout << "EventLoop::wakeup() writes " << n << " bytes instead of 8" << std::endl;
        //abort();
    }
}

void EventLoop::HandleRead()
{
    uint64_t one = 1;
    auto n = ::read(wakeup_fd, &one, sizeof(one));

    if (n != sizeof(one))
    {
        std::cout << "EventLoop::handleRead() writes " << n << " bytes instead of 8" << std::endl;
        //abort();
    }
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

void EventLoop::DoPendingFunctors()
{
    calling_pending_functors = true;

    std::vector<Functor> functors;
    {
        std::lock_guard lock(mutex_);
        functors.swap(pending_functors);
    }

    for (auto& f : functors)
    {
        f();
    }

    calling_pending_functors = false;
}
