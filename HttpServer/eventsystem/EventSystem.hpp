#pragma once

#include <memory>
#include <deque>
#include "IEvent.hpp"
#include "EventDispatcher.hpp"

class EventSystem
{
public:
    EventSystem() : threads_(NUM_OF_THREADS)
    {
        threads_.accept([this](){});
    }

    void post(const std::shared_ptr<IEvent>& event)
    {
        events_.emplace_back(event);
    }

    template<typename EventType, typename... Args>
    void post(Args&&... args)
    {
        events_.emplace_back(std::make_shared<EventType>(std::forward<Args>(args)...));
    }

    void processEvents()
    {
        while()
    }

private:
    EventDispatcher eventDispatcher_;
};
