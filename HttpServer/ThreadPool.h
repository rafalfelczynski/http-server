#pragma once

#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <functional>

#include "Observer/IPublisher.h"

namespace http
{
struct IJob
{
    virtual ~IJob() = default;
    virtual void execute() = 0;
};

class WorkerThread : public observer::IPublisher<>
{
    enum class State
    {
        Idle,
        Running,
        Stopped
    };

public:
    WorkerThread();
    void acceptJob(std::unique_ptr<IJob>&& job);
    void acceptJob(std::function<void()> job);

    void run();
    void wakeUp();
    void join();
    void stop();
    bool isRunning();
    bool isIdle();

private:
    void goToSleep();
    void doJobs();
    void doOneJob();
    std::unique_ptr<std::thread> thread_;
    State state = State::Idle;
    std::queue<std::unique_ptr<IJob>> jobs_;
    std::queue<std::function<void()>> jobs2_;
    std::condition_variable monitor_;
    std::mutex mutex_;
    bool isRunning_ = true;
};

class ThreadPool
{
public:
    ThreadPool(unsigned size);
    void delegate();

private:
    std::vector<std::unique_ptr<std::thread>> pool_;
};
}  // namespace http