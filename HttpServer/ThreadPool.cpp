#include "ThreadPool.h"

#include <iostream>

#include "ThirdParty/Timer.hpp"

namespace http
{
WorkerThread::WorkerThread()
    : isRunning_{true}, isExecutingJob_{false}, thread_(std::make_unique<std::thread>([this]() { run(); }))
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
    if (isRunning())
    {
        jobs_.enqueue(std::make_unique<AnonymousJob>(std::move(job)));
        wakeUp();
    }
}

void WorkerThread::run()
{
    while (isRunning_)
    {
        state = State::Running;
        doJobs();
        goToSleep();
    }
    // ensureCanFinish();
    state = State::Stopped;
}

void WorkerThread::wakeUp()
{
    monitor_.notify_all();
}

void WorkerThread::kill()
{
    setRunning(false);
    jobs_.clear();
    wakeUp();
    if (thread_->joinable())
    {
        thread_->detach();
    }
}

bool WorkerThread::isRunning()
{
    std::unique_lock lock(mutex_);
    return isRunning_;
}

bool WorkerThread::hasPendingJobs()
{
    return !jobs_.empty();
}

void WorkerThread::join()
{
    if (thread_->joinable())
    {
        thread_->join();
    }
}

unsigned WorkerThread::getNumOfPendingJobs() const
{
    const auto numOfJobs = static_cast<unsigned>(jobs_.size());
    return isExecutingJob_ ? numOfJobs+1 : numOfJobs;
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
    while (!jobs_.empty())
    {
        doOneJob();
    }
}

void WorkerThread::doOneJob()
{
    auto job = jobs_.dequeue();
    if (job)
    {
        isExecutingJob_ = true;
        std::cout << "job->execute()" << std::endl;
        (*job)->execute();
    }
    isExecutingJob_ = false;
}

void WorkerThread::ensureCanFinish()
{
    doJobs();
}

ThreadPool::ThreadPool(unsigned size)
    : readyToBeDestroyed_(false)
    , threadsToBeDestroyedCounter_(size)
{
    if (size <= 0)
    {
        throw std::invalid_argument("Thread pool size should be greater than 0");
    }

    threads_.reserve(size);
    for (unsigned i = 0; i < size; i++)
    {
        threads_.emplace_back(std::make_unique<WorkerThread>());
    }
}

ThreadPool::~ThreadPool() 
{
    freeThreads();
}

void ThreadPool::process(std::unique_ptr<IJob> job)
{
    getFreeThread()->acceptJob(std::move(job));
}

void ThreadPool::process(std::function<void()> job)
{
    getFreeThread()->acceptJob(std::move(job));
}

void ThreadPool::freeThreads()
{
    for (auto& thread : threads_)
    {
        if (thread->hasPendingJobs())
        {
            std::cout << "thread has pending jobes" << std::endl;
            timer::SingleShotTimer::call(5000, [this, &thread]() {
                std::cout << "killing thread" << std::endl;
                thread->kill();
                std::cout << "thread killed" << std::endl;
                this->threadsToBeDestroyedCounter_--;
                this->threadsToBeDestroyedMonitor_.notify_all();
            });
        }
        else
        {
            std::cout << "thread has no pending jobes. killing" << std::endl;
            thread->kill();
            std::cout << "thread killed" << std::endl;
        }
    }
    timer::SingleShotTimer::call(5000, [this]() {
        std::unique_lock lock(threadsToBeDestroyedMutex_);
        std::cout << "starting to wait for all threads to finish" << std::endl;
        threadsToBeDestroyedMonitor_.wait(lock, [this]() { return threadsToBeDestroyedCounter_ == 0; });
        this->readyToBeDestroyed_ = true;
        std::cout << "cleared list of threads" << std::endl;
        threads_.clear();
    });
}

void ThreadPool::joinAll()
{
    for (auto& thr : threads_)
    {
        thr->join();
    }
}

std::unique_ptr<WorkerThread>& ThreadPool::getFreeThread()
{
    return *std::min_element(threads_.begin(), threads_.end(), [](auto& thr1, auto& thr2) {
        return thr1->getNumOfPendingJobs() < thr2->getNumOfPendingJobs();
    });
}
}  // namespace http