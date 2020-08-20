#pragma once
#include "base/NonCopyable.h"
#include <functional>

class EventLoop;

class Channel : NonCopyable
{
public:
    using EventCallback = std::function<void()>;

public:
    Channel(EventLoop* loop, int fd);

    void HandleEvent();

    void SetReadCallback(EventCallback cb) { read_cb = std::move(cb); }
    void SetWriteCallback(EventCallback cb) { write_cb = std::move(cb); }
    void SetErrorCallback(EventCallback cb) { error_cb = std::move(cb); }

    int Fd() const { return fd; }
    int Events() const { return events; }
    void SetREvents(int revent) { revents = revent; }
    bool IsNoneEvent() const { return events == none_event; }

    void EnableReading()
    {
        events |= read_event;
        Update();
    }

    // for Poller
    int Index() const { return index; }
    void SetIndex(int idx) { index = idx; }
    EventLoop* OwnerLoop() const { return loop; }

private:
    void Update();

private:
    static const int none_event;
    static const int read_event;
    static const int write_event;

private:
    EventLoop* loop;
    const int fd;
    int events;
    int revents;
    int index;

    EventCallback read_cb;
    EventCallback write_cb;
    EventCallback error_cb;
};