#include "ThreadPool.h"

#include <iostream>

namespace http
{
WorkerThread::WorkerThread() : thread_(std::make_unique<std::thread>([this]() { run(); })) {}

void WorkerThread::acceptJob(std::unique_ptr<IFunctor> job)
{
    //auto jobPtr = std::unique_ptr<std::remove_pointer_t<decltype(job)>>(job);
    jobs_.push(std::move(job));
    wakeUp();
}

void WorkerThread::run()
{
    while (isRunning_)
    {
        std::cout << "running" << std::endl;
        doJobs();
        goToSleep();
    }
    state = State::Stopped;
}

void WorkerThread::wakeUp() { monitor_.notify_all(); }

void WorkerThread::join() { thread_->join(); }

void WorkerThread::stop()
{
    isRunning_ = false;
    wakeUp();
}

void WorkerThread::goToSleep()
{
    std::unique_lock lock(mutex_);
    monitor_.wait(lock);
}

void WorkerThread::doJobs()
{
    while (!jobs_.empty())
    {
        doOneJob();
    }
}

void WorkerThread::doOneJob()
{
    state = State::Running;
    auto& job = jobs_.front();
    (*job)();
    jobs_.pop();
    state = State::Idle;
}

ThreadPool::ThreadPool(unsigned size) { pool_.reserve(size); }
}  // namespace http