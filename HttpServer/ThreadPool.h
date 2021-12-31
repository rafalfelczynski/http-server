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
#include "SafeQueue/SafeQueue.h"

namespace http
{
struct IJob
{
    virtual ~IJob() = default;
    virtual void execute() = 0;
};

class WorkerThread
{
    enum class State
    {
        Idle,
        Running,
        Stopped
    };

public:
    WorkerThread();
    void acceptJob(std::unique_ptr<IJob> job);
    void acceptJob(std::function<void()> job);

    void run();
    void wakeUp();
    void waitForThreadToFinishJobs();
    void kill();
    bool isRunning();
    bool hasPendingJobs();
    void join();

private:
    void setRunning(bool isRunning);
    void goToSleep();
    void doJobs();
    void doOneJob();
    void ensureCanFinish();

    std::unique_ptr<std::thread> thread_;
    State state = State::Idle;
    collections::SafeQueue<std::unique_ptr<IJob>> jobs_;
    std::condition_variable monitor_;
    std::mutex mutex_;
    bool isRunning_ = true;
};

class ThreadPool
{
public:
    ThreadPool(unsigned size);
    void process(std::unique_ptr<IJob> job);
    void process(std::function<void()> job);
    void freeThreads();
    void joinAll();

private:
    std::vector<std::unique_ptr<WorkerThread>> threads_;
};
}  // namespace http