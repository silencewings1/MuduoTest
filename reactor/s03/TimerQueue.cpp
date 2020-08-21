#include "TimerQueue.h"
#include "EventLoop.h"
#include "Timer.h"
#include "TimerId.h"
#include <iostream>
#include <string.h>
#include <sys/timerfd.h>
#include <unistd.h>

namespace
{
int CreateTimerFd()
{
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
                                   TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0)
    {
        std::cout << "Failed in timerfd_create\n";
        abort();
    }

    return timerfd;
}

struct timespec TimeSinceNow(const TimeStamp& when)
{
    using namespace std::chrono;
    using namespace std::chrono_literals;

    auto dur = when.DurationSinceNow();
    if (dur < 100ms)
        dur = 100ms;

    struct timespec ts;
    ts.tv_sec = duration_cast<seconds>(dur).count();
    ts.tv_nsec = duration_cast<nanoseconds>(dur - seconds(ts.tv_sec)).count();

    return ts;
}

void ResetTimerFd(int timer_fd, const TimeStamp& expiration)
{
    struct itimerspec newValue;
    bzero(&newValue, sizeof(newValue));

    newValue.it_value = TimeSinceNow(expiration);
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
    auto n = ::read(timer_fd, &howmany, sizeof(howmany));
    // std::cout << "TimerQueue::handleRead() " << howmany
    //           << " at " << now.ToString() << std::endl;
    if (n != sizeof(howmany))
    {
        std::cout << "TimerQueue::handleRead() reads " << n << " bytes instead of 8" << std::endl;
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
}

TimerId TimerQueue::AddTimer(const TimerCallback& timer_cb, TimeStamp when, Duration interval)
{
    auto timer = std::make_shared<Timer>(timer_cb, when, interval);
    loop->RunInLoop([&]() { this->AddTimerInLoop(timer); });

    return TimerId(timer.get());
}

void TimerQueue::AddTimerInLoop(std::shared_ptr<Timer> timer)
{
    loop->AssertInLoopThread();

    bool earliest_changed = Insert(timer);
    if (earliest_changed)
    {
        ResetTimerFd(timer_fd, timer->Expiration());
    }
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
    TimerList::iterator it = timers.begin();
    for (; it != timers.end(); ++it)
    {
        if (it->first >= now)
            break;
    }

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

bool TimerQueue::Insert(std::shared_ptr<Timer> timer)
{
    auto when = timer->Expiration();

    bool earliest_changed = timers.empty() || when < timers.begin()->first;
    timers.insert(std::make_pair(when, timer));

    return earliest_changed;
}