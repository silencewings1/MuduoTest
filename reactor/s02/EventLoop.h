#pragma once
#include "base/NonCopyable.h"
#include "thread/Thread.h"
#include <memory>
#include <vector>

class Channel;
class Poller;

class EventLoop : NonCopyable
{
public:
    EventLoop();
    ~EventLoop();

    void Loop();
    void Quit();

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
};