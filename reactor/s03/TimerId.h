#pragma once

class Timer;

class TimerId
{
public:
    explicit TimerId(Timer* timer)
        : timer(timer)
    {
    }

private:
    Timer* timer;
};