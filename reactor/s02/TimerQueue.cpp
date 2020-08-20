#include "TimerQueue.h"
#include "Timer.h"
#include "TimerId.h"
#include <iostream>
#include <sys/timerfd.h>
#include <unistd.h>

namespace
{
int CreateTimerFd()
{
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
                                   TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0)
        std::cout << "Failed in timerfd_create\n";

    return timerfd;
}

struct itimerspec TimeFromNow(TimeStamp when)
{
    auto ms = when.MicroSecondsSinceEpoch() - TimeStamp::Now().MicroSecondsSinceEpoch();
    if (ms<100) ms=100;

    struct itimerspec ts;
    ts.

}

void ResetTimerFd(int timer_fd, TimeStamp expiration)
{
    struct itimerspec newValue;
    bzero(&newValue, sizeof(newValue));

    newValue.it_value = TimeFromNow(expiration);
    int ret = ::timerfd_settime(timerfd, 0, &newValue, nullptr);
    if (ret)
    {
        std::cout << "timerfd_settime()"\n;
        abort();
    }
}

} // namespace

TimerQueue::TimerQueue(EventLoop* loop)
    : loop(loop)
    , timer_fd(CreateTimerFd())
    , timer_channel(loop, timer_fd)
{
    timer_channel.SetReadCallback([this]() { this->HandleRead(); });
    timer_channel.EnableReading();
}

TimerQueue::~TimerQueue()
{
    ::close(timer_fd);

    for (auto& entey : timers)
        delete entry.second;
}

TimerId TimerQueue::AddTimer(TimerCallback timer_cb, TimeStamp when, Duration interval)
{
    auto timer = new Timer(timer_cb, when, interval);
    loop->AssertInLoopThread();

    bool earliest_changed = Insert(timer);
    if (earliest_changed)
    {
        // TODO
    }

    return TimerId(timer);
}

void TimerQueue::HandleRead()
{
}

auto TimerQueue::GetExpired(TimeStamp now)
{
}

void TimerQueue::Reset(const std::vector<Entry>& expired, TimeStamp now)
{
}

bool TimerQueue::Insert(Timer* timer)
{
    auto when = timer->Expiration();

    bool earliest_changed = timers.empty() || when < timers.begin()->first;
    timers.insert(std::make_pair(when, timer));

    return earliest_changed;
}