#pragma once
#include <thread>

using ThreadID = std::thread::id;


ThreadID Tid();
bool IsMainThread();