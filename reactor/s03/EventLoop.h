#pragma once
#include "Callbacks.h"
#include "TimerId.h"
#include "base/NonCopyable.h"
#include "datatime/TimeStamp.h"
#include "thread/Thread.h"
#include <memory>
#include <vector>

class Channel;
class Poller;
class TimerQueue;

class EventLoop : NonCopyable
{
public:
    EventLoop();
    ~EventLoop();

    void Loop();
    void Quit();

    TimeStamp PollReturnTime() const { return poll_return_time; }

    TimerId RunAt(const TimeStamp& when, TimerCallback cb);
    TimerId RunAfter(const Duration& delay, TimerCallback cb);
    TimerId RunEvery(const Duration& interval, TimerCallback cb);

    void UpdateChannel(Channel* channel);

    void AssertInLoopThread();
    bool IsInLoopThread() const;

private:
    void AbortNotInLoopThread();

public:
    bool looping;
    bool quit;
    const ThreadID thread_id;
    std::unique_ptr<Poller> poller;
    std::unique_ptr<TimerQueue> timer_queue;
    TimeStamp poll_return_time;
};