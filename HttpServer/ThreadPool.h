#pragma once

#include <condition_variable>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "Observer/IPublisher.h"
#include "SafeQueue/SafeQueue.h"

namespace http
{
struct IJob
{
    virtual ~IJob() = default;
    virtual void execute() = 0;
};

class ThreadPool;

class WorkerThread
{
    friend class http::ThreadPool;
    enum class State
    {
        Idle,
        Running,
        Stopped
    };

public:
    ~WorkerThread();
    void acceptJob(std::unique_ptr<IJob> job);
    void acceptJob(std::function<void()> job);

    void run();
    void wakeUp();
    void kill();
    bool isRunning();
    bool hasPendingJobs();
    void join();
    unsigned getNumOfPendingJobs() const;

private:
    WorkerThread();
    void setRunning(bool isRunning);
    void goToSleep();
    void doJobs();
    void doOneJob();
    void ensureCanFinish();

    bool isRunning_;
    bool isExecutingJob_;
    State state = State::Idle;
    collections::SafeQueue<std::unique_ptr<IJob>> jobs_;
    std::unique_ptr<std::condition_variable> monitor_;
    std::unique_ptr<std::mutex> mutex_;
    std::unique_ptr<std::thread> thread_;
    // unsigned exitTimeoutMs_;
    std::unique_ptr<std::condition_variable> exitMonitor_;
    std::unique_ptr<std::mutex> exitMutex_;
    std::thread::id id_;
};

class ThreadPool
{
public:
    ThreadPool(unsigned size);
    ~ThreadPool();
    void init();
    void process(std::unique_ptr<IJob> job);
    void process(std::function<void()> job);
    void freeThreads();
    void joinAll();

private:
    WorkerThread& getFreeThread();
    std::vector<std::unique_ptr<WorkerThread>> threads_;
    bool readyToBeDestroyed_;
    unsigned numOfThreads_;
};
}  // namespace http