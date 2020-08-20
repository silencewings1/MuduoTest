#include "TimerQueue.h"
#include "Timer.h"
#include "TimerId.h"
#include <iostream>
#include <sys/timerfd.h>
#include <unistd.h>
#include <string.h>
#include "EventLoop.h"

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

struct timespec TimeFromNow(const TimeStamp& when)
{
    using namespace std::chrono;

    auto dur = when.DurationSinceNow();

    struct timespec ts;
    ts.tv_sec = duration_cast<seconds>(dur).count();
    ts.tv_nsec = duration_cast<nanoseconds>(dur).count();

    return ts;
}

void ResetTimerFd(int timer_fd, const TimeStamp& expiration)
{
    struct itimerspec newValue;
    bzero(&newValue, sizeof(newValue));

    newValue.it_value = TimeFromNow(expiration);
    int ret = ::timerfd_settime(timer_fd, 0, &newValue, nullptr);
    if (ret)
    {
        std::cout << "timerfd_settime()\n";
        abort();
    }
}

void ReadTimerFd(int timer_fd, const TimeStamp& now)
{
    uint64_t howmany;
    ::read(timer_fd, &howmany, sizeof(howmany));
    std::cout << "TimerQueue::handleRead() " << howmany 
              << " at " << now.ToString() << std::endl;
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

    for (auto& entry : timers)
    {
        delete entry.second;
    }
}

TimerId TimerQueue::AddTimer(TimerCallback timer_cb, TimeStamp when, Duration interval)
{
    auto timer = new Timer(timer_cb, when, interval);
    loop->AssertInLoopThread();

    bool earliest_changed = Insert(timer);
    if (earliest_changed)
    {
        ResetTimerFd(timer_fd, timer->Expiration());
    }

    return TimerId(timer);
}

void TimerQueue::HandleRead()
{
    loop->AssertInLoopThread();

    auto now = TimeStamp::Now();
    ReadTimerFd(timer_fd, now);

    auto expired = GetExpired(now);
    for (auto& entry : expired)
    {
        entry.second->Run();
    }

    Reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::GetExpired(TimeStamp now)
{
    Entry sentry = std::make_pair(std::move(now), reinterpret_cast<Timer*>(UINTPTR_MAX));
    auto it = timers.lower_bound(sentry);

    std::vector<TimerQueue::Entry> expired;
    std::copy(timers.begin(), it, std::back_inserter(expired));
    timers.erase(timers.begin(), it);

    return expired;
}

void TimerQueue::Reset(const std::vector<Entry>& expired, TimeStamp now)
{
    for (auto& exp : expired)
    {
        if (exp.second->Repeat())
        {
            exp.second->Restart(now);
            Insert(exp.second);
        }
        else
        {
            delete exp.second;
        }

        if (!timers.empty())
        {
            auto next_expired = timers.begin()->second->Expiration();
            if (next_expired.Vaild())
            {
                ResetTimerFd(timer_fd, next_expired);
            }
        }
    }
}

bool TimerQueue::Insert(Timer* timer)
{
    auto when = timer->Expiration();

    bool earliest_changed = timers.empty() || when < timers.begin()->first;
    timers.insert(std::make_pair(when, timer));

    return earliest_changed;
}