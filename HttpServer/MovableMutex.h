#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>

namespace http
{
class MovableMutex
{
public:
    MovableMutex()
        : mutex_(std::make_unique<std::mutex>())
    {
    }

    operator std::mutex&()
    {
        return *mutex_;
    }

    std::mutex& operator*()
    {
        return *mutex_;
    }

private:
    std::unique_ptr<std::mutex> mutex_;
};

class MovableMonitor
{
public:
    MovableMonitor()
        : monitor_(std::make_unique<std::condition_variable>())
    {
    }

    operator std::condition_variable&()
    {
        return *monitor_;
    }

    std::condition_variable& operator*()
    {
        return *monitor_;
    }

private:
    std::unique_ptr<std::condition_variable> monitor_;
};
}  // namespace http
