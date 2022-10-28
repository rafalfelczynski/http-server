#include "ThreadPool.h"

#include <cassert>
#include <iostream>

#include "ThirdParty/Timer.hpp"

using namespace std::chrono;

namespace http
{
WorkerThread::WorkerThread()
    : isRunning_{true}
    , isExecutingJob_{false}
    , monitor_(std::make_unique<std::condition_variable>())
    , mutex_(std::make_unique<std::mutex>())
    , thread_(std::make_unique<std::thread>([this]() { run(); }))
    , exitMonitor_(std::make_unique<std::condition_variable>())
    , exitMutex_(std::make_unique<std::mutex>())
{
    id_ = thread_->get_id();
    std::cout << "created thread " << id_ << std::endl;
}

WorkerThread::~WorkerThread()
{
    std::cout << "worker thread destr " << std::endl;
    // if (exitTimeoutMs_ == 0)
    // {
    //     //
    //     kill();
    // }
    // else
    // {
    //     std::unique_lock lock(*exitMutex_);
    //     exitMonitor_->wait_for(lock, milliseconds(exitTimeoutMs_));
    //     kill();
    // }

    // notify threadDestroyed
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
        void execute() override
        {
            functor_();
        }
        AnonymousJob(std::function<void()>&& functor)
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
        exitMonitor_->notify_all();
    }
    // ensureCanFinish();
    state = State::Stopped;
}

void WorkerThread::wakeUp()
{
    monitor_->notify_all();
}

void WorkerThread::kill()
{
    std::cout << "Worker::kill() started " << id_ << std::endl;
    setRunning(false);
    jobs_.clear();
    wakeUp();
    if (thread_->joinable())
    {
        thread_->join();
    }
    std::cout << "Worker::kill() ended " << id_ << std::endl;
}

bool WorkerThread::isRunning()
{
    std::unique_lock lock(*mutex_);
    return isRunning_;
}

bool WorkerThread::hasPendingJobs()
{
    return !jobs_.empty() || isExecutingJob_;
}

void WorkerThread::join()
{
    if (thread_->joinable())
    {
        wakeUp();
        std::cout << "Waiting on exitMonitor " << hasPendingJobs() << std::endl;
        std::unique_lock lock(*exitMutex_);
        exitMonitor_->wait(lock, [this]() { return !hasPendingJobs(); });
        std::cout << "woke up before joining(killing) thread" << hasPendingJobs() << std::endl;
        kill();
        std::cout << "finished joining(killing) std::thread " << hasPendingJobs() << std::endl;
    }
}

unsigned WorkerThread::getNumOfPendingJobs() const
{
    const unsigned numOfJobs = static_cast<unsigned>(jobs_.size());
    return isExecutingJob_ ? numOfJobs + 1 : numOfJobs;
}

void WorkerThread::setRunning(bool isRunning)
{
    std::unique_lock lock(*mutex_);
    isRunning_ = isRunning;
}

void WorkerThread::goToSleep()
{
    std::unique_lock lock(*mutex_);
    state = State::Idle;
    if (isRunning_)
    {
        monitor_->wait(lock);
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
        (*job)->execute();
    }
    isExecutingJob_ = false;
    exitMonitor_->notify_all();
}

void WorkerThread::ensureCanFinish()
{
    doJobs();
}

ThreadPool::ThreadPool(unsigned size)
    : readyToBeDestroyed_(false)
    , numOfThreads_(size)
{
    if (size == 0)
    {
        throw std::logic_error("Thread pool size should be greater than 0");
    }
}

void ThreadPool::init()
{
    threads_.reserve(numOfThreads_);
    for (unsigned i = 0; i < numOfThreads_; i++)
    {
        threads_.emplace_back(std::unique_ptr<WorkerThread>(new WorkerThread()));
    }
}

ThreadPool::~ThreadPool()
{
    freeThreads();
}

void ThreadPool::process(std::unique_ptr<IJob> job)
{
    if (threads_.empty())
    {
        throw std::logic_error("Call ThreadPool::init() before posting a job");
    }
    getFreeThread().acceptJob(std::move(job));
}

void ThreadPool::process(std::function<void()> job)
{
    if (threads_.empty())
    {
        throw std::logic_error("Call ThreadPool::init() before posting a job");
    }
    getFreeThread().acceptJob(std::move(job));
}

void ThreadPool::freeThreads()
{
    if (threads_.empty())
    {
        return;
    }
    std::mutex mutex;
    std::atomic_uint threadsCounter(threads_.size());
    std::condition_variable monitor;
    for (auto& thread : threads_)
    {
        if (thread->hasPendingJobs())
        {
            std::cout << "thread has pending jobes" << std::endl;
            timer::SingleShotTimer::call(2000, [&thread, &monitor, &threadsCounter]() {
                std::cout << "killing thread" << std::endl;
                thread->kill();
                std::cout << "thread killed" << std::endl;
                threadsCounter--;
                monitor.notify_all();
            });
        }
        else
        {
            std::cout << "thread has no pending jobes. killing" << std::endl;
            thread->kill();
            threadsCounter--;
            monitor.notify_all();
            std::cout << "thread killed" << std::endl;
        }
    }

    std::unique_lock lock(mutex);
    std::cout << "starting to wait for all threads to finish" << std::endl;
    monitor.wait(lock, [&threadsCounter]() { return threadsCounter == 0; });
    std::cout << "All threads in that pool should be freed now" << std::endl;
}

void ThreadPool::joinAll()
{
    for (auto i = threads_.size(); i > 0; i--)
    {
        std::cout << "thread pool join thread" << std::endl;
        threads_.back()->join();
        threads_.pop_back();
    }
}

WorkerThread& ThreadPool::getFreeThread()
{
    return **std::min_element(threads_.begin(), threads_.end(), [](auto& thr1, auto& thr2) {
        return thr1->getNumOfPendingJobs() < thr2->getNumOfPendingJobs();
    });
}
}  // namespace http