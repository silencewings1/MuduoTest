#pragma once
#include "Callbacks.h"
#include "datatime/TimeStamp.h"

class Timer
{
public:
    Timer(TimerCallback timer_cb, TimeStamp when, Duration interval)
        : cb(std::move(timer_cb))
        , expiration(std::move(when))
        , duration(std::move(interval))
        , repeat(duration.count() > 0)
    {
    }

    void Run() const { cb(); }

    TimeStamp Expiration() const { return expiration; }
    bool Repeat() const { return repeat; }

    void Restart(TimeStamp now)
    {
        expiration = repeat ? AddTime(now, duration) : TimeStamp::Invaild();
    }

private:
    const TimerCallback cb;
    TimeStamp expiration;
    const Duration duration;
    const bool repeat;
};