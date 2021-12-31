#include "ThreadPool.h"

#include <iostream>

namespace http
{
WorkerThread::WorkerThread()
    : thread_(std::make_unique<std::thread>([this]() { run(); }))
{
}

void WorkerThread::acceptJob(std::unique_ptr<IJob> job)
{
    if (isRunning())
    {
        jobs_.enqueue(std::move(job));
        wakeUp();
    }
}

void WorkerThread::acceptJob(std::function<void()> job)
{
    struct AnonymousJob : public IJob
    {
        std::function<void()> functor_;
        inline void execute() override
        {
            functor_();
        }
        inline AnonymousJob(std::function<void()>&& functor)
            : functor_(std::move(functor))
        {
        }
    };
    acceptJob(std::make_unique<AnonymousJob>(std::move(job)));
}

void WorkerThread::run()
{
    while (isRunning_)
    {
        goToSleep();
        std::cout << "running" << std::endl;
        state = State::Running;
        doJobs();
    }
    ensureCanFinish();
    state = State::Stopped;
}

void WorkerThread::wakeUp()
{
    monitor_.notify_all();
}

void WorkerThread::waitForThreadToFinishJobs()
{
    setRunning(false);
    wakeUp();
    thread_->join();
    ;
}

void WorkerThread::kill()
{
    setRunning(false);
    jobs_.clear();
    wakeUp();
    thread_->join();
}

bool WorkerThread::isRunning()
{
    std::unique_lock lock(mutex_);
    return isRunning_;
}

bool WorkerThread::hasPendingJobs()
{
    return !jobs_.isEmpty();
}

void WorkerThread::join() 
{
    thread_->join();
}

void WorkerThread::setRunning(bool isRunning)
{
    std::unique_lock lock(mutex_);
    isRunning_ = isRunning;
}

void WorkerThread::goToSleep()
{
    std::unique_lock lock(mutex_);
    state = State::Idle;
    if (isRunning_)
    {
        monitor_.wait(lock);
    }
}

void WorkerThread::doJobs()
{
    while (!jobs_.isEmpty())
    {
        doOneJob();
    }
}

void WorkerThread::doOneJob()
{
    auto job = jobs_.dequeue();
    if (job)
    {
        job->execute();
    }
}

void WorkerThread::ensureCanFinish()
{
    doJobs();
}

ThreadPool::ThreadPool(unsigned size)
{
    threads_.reserve(size);
    for (unsigned i = 0; i < size; i++)
    {
        threads_.emplace_back(std::make_unique<WorkerThread>());
    }
}

void ThreadPool::process(std::unique_ptr<IJob> job)
{
}

void ThreadPool::process(std::function<void()> job)
{
    // choose next free thread
    threads_[0]->acceptJob(std::move(job));
}

void ThreadPool::freeThreads()
{
    for (auto& thread : threads_)
    {
        if (thread->hasPendingJobs())
        {
            // Give a thread some time (5 seconds) to finish. Kill after timeout
            // Set timer, waitForThreadToFinishJobs()
        }
        else
        {
            thread->kill();
        }
    }
}

void ThreadPool::joinAll() 
{
    for(auto&thr : threads_)
    {
        thr->join();
    }
}
}  // namespace http