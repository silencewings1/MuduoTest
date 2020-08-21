#include "EventLoopThread.h"
#include "EventLoop.h"

EventLoopThread::EventLoopThread()
    : loop(nullptr)
{
}

EventLoopThread::~EventLoopThread()
{
    if (thread_.joinable())
        thread_.join();

    loop->Quit();
}

EventLoop* EventLoopThread::StartLoop()
{
    thread_ = std::thread([this]() { this->Task(); });

    {
        std::unique_lock lock(mutex_);
        cond_.wait(lock, [this]() { return loop != nullptr; });
    }

    return loop;
}

void EventLoopThread::Task()
{
    EventLoop new_loop;

    {
        std::unique_lock lock(mutex_);
        loop = &new_loop;
        cond_.notify_one();
    }

    loop->Loop();
}