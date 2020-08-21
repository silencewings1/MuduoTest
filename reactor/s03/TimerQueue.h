#pragma once
#include "Callbacks.h"
#include "Channel.h"
#include "base/NonCopyable.h"
#include "datatime/TimeStamp.h"
#include <set>
#include <vector>
#include <memory>

class Timer;
class TimerId;
class EventLoop;

class TimerQueue : NonCopyable
{
private:
    using Entry = std::pair<TimeStamp, std::shared_ptr<Timer>>;
    using TimerList = std::set<Entry>;

public:
    TimerQueue(EventLoop* loop);
    ~TimerQueue();

    TimerId AddTimer(const TimerCallback& timer_cb,
                     TimeStamp when,
                     Duration interval = std::chrono::seconds(0));

private:
    void HandleRead();

    std::vector<Entry> GetExpired(TimeStamp now);
    void Reset(const std::vector<Entry>& expired, TimeStamp now);
    bool Insert(std::shared_ptr<Timer> timer);

    void AddTimerInLoop(std::shared_ptr<Timer> timer);

private:
    EventLoop* loop;

    const int timer_fd;
    Channel timer_channel;
    TimerList timers;
};