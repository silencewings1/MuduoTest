#pragma once
#include "Callbacks.h"
#include "Channel.h"
#include "base/NonCopyable.h"
#include "datatime/TimeStamp.h"
#include <set>
#include <vector>

class Timer;
class TimerId;
class EventLoop;

class TimerQueue : NonCopyable
{
private:
    using Entry = std::pair<TimeStamp, Timer*>;
    using TimerList = std::set<Entry>;

public:
    TimerQueue(EventLoop* loop);
    ~TimerQueue();

    TimerId AddTimer(TimerCallback timer_cb,
                     TimeStamp when,
                     Duration interval = std::chrono::seconds(0));

private:
    void HandleRead();

    auto GetExpired(TimeStamp now);
    void Reset(const std::vector<Entry>& expired, TimeStamp now);
    bool Insert(Timer* timer);

private:
    EventLoop* loop;
    const int timer_fd;

    Channel timer_channel;
    TimerList timers;
};