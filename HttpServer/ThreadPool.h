#pragma once

#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace http
{
struct IFunctor
{
    virtual ~IFunctor() = default;
    virtual void operator()() = 0;
};

class JobMaker;

template<typename Func>
class Job : public IFunctor
{
    friend class JobMaker;

public:
    inline void operator()() override { functor_(); }

private:
    template<typename FuncType>
    inline Job(FuncType&& functor) : functor_(std::forward<FuncType>(functor))
    {
    }
    Func functor_;
};

class JobMaker
{
public:
    template<typename Func>
    inline static std::unique_ptr<Job<std::remove_reference_t<Func>>> make(Func&& functor)
    {
        using JobType = std::remove_reference_t<Func>;
        auto* rawPtr = new Job<JobType>(std::forward<Func>(functor));
        return std::unique_ptr<Job<JobType>>(rawPtr);
    }
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
    void acceptJob(std::unique_ptr<IFunctor> job);

    void run();
    void wakeUp();
    void join();
    void stop();

private:
    void goToSleep();
    void doJobs();
    void doOneJob();
    std::unique_ptr<std::thread> thread_;
    State state = State::Idle;
    std::queue<std::unique_ptr<IFunctor>> jobs_;
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