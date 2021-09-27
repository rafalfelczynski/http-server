#pragma once
#include <mutex>
#include <queue>

namespace collections
{
template<typename Type>
class SafeQueue
{
public:
    inline bool isEmpty()
    {
        std::lock_guard lock(mutex_);
        return queue_.empty();
    }

    inline size_t size()
    {
        std::lock_guard lock(mutex_);
        return queue_.size();
    }

    template<typename Element>
    inline void enqueue(Element&& element)
    {
        std::lock_guard lock(mutex_);
        queue_.push(std::forward<Element>(element));
    }

    inline Type dequeue()
    {
        std::lock_guard lock(mutex_);
        auto element{std::move(queue_.front())};
        queue_.pop();
        return element;
    }

    inline void clear()
    {
        std::lock_guard lock(mutex_);
        queue_ = std::queue<Type>();
    }

private:
    std::queue<Type> queue_;
    std::mutex mutex_;
};
}  // namespace collections
