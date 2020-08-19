#include "EventLoop.h"
#include <iostream>
#include <poll.h>
#include <cassert>

thread_local EventLoop* loop_in_this_thread = nullptr;

EventLoop::EventLoop()
    : looping(false)
    , thread_id(Tid())
{
    std::cout << "EventLoop created " << this
              << " in thread " << thread_id << std::endl;
    if (loop_in_this_thread)
    {
        std::cout << "Another EventLoop " << loop_in_this_thread
                  << " exists in this thread " << thread_id << std::endl;
        abort();
    }


    loop_in_this_thread = this;
}

EventLoop::~EventLoop()
{
    assert(!looping);
    loop_in_this_thread = nullptr;
}

void EventLoop::Loop()
{
    assert(!looping);
    AssertInLoopThread();

    looping = true;

    ::poll(nullptr, 0, 1 * 1000);
    std::cout << "EventLoop " << this << " stop looping" << std::endl;

    looping = false;
}

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