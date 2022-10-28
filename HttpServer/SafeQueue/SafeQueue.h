#pragma once
#include <deque>
#include <mutex>
#include <optional>

namespace collections
{
template<typename Type>
class SafeQueue
{
public:
    SafeQueue()
        : mutex_(std::make_unique<std::mutex>())
    {
    }

    inline bool empty() const
    {
        std::lock_guard lock(*mutex_);
        return queue_.empty();
    }

    inline size_t size() const
    {
        std::lock_guard lock(*mutex_);
        return queue_.size();
    }

    template<typename Element>
    inline void enqueue(Element&& element)
    {
        std::lock_guard lock(*mutex_);
        queue_.emplace_back(std::forward<Element>(element));
    }

    inline std::optional<Type> dequeue()
    {
        std::lock_guard lock(*mutex_);
        if (queue_.empty())
        {
            return {};
        }
        auto element{std::move(queue_.front())};
        queue_.pop_front();
        return element;
    }

    inline void clear()
    {
        std::lock_guard lock(*mutex_);
        queue_.clear();
    }

private:
    std::deque<Type> queue_;
    mutable std::unique_ptr<std::mutex> mutex_;
};
}  // namespace collections
