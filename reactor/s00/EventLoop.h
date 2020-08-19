#pragma once
#include "base/NonCopyable.h"
#include "thread/Thread.h"

class EventLoop : NonCopyable
{
public:
    EventLoop();
    ~EventLoop();

    void Loop();

    void AssertInLoopThread();
    bool IsInLoopThread() const;

private:
    void AbortNotInLoopThread();

private:
    bool looping;
    ThreadID thread_id;
};