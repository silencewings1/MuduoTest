#include "Thread.h"
#include <sys/syscall.h>
#include <unistd.h>

ThreadID Tid()
{
    return std::this_thread::get_id();
}

bool IsMainThread()
{
    return ::getpid() == static_cast<pid_t>(::syscall(SYS_gettid));
}