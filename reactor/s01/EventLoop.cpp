#include "EventLoop.h"
#include <iostream>
#include <cassert>


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