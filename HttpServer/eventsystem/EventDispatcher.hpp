#pragma once

#include <deque>
#include <map>
#include <memory>
#include <optional>

#include "SafeQueue/SafeQueue.h"

#include "IEvent.hpp"
#include "IEventListener.hpp"
#include "ThreadPool.h"

struct EventDispatcher
{
public:
    EventDispatcher()
        : dispatcherThread_(1)
    {
        dispatcherThread_.init();
    }

    void listen(IEventListener* listener, EventType type)
    {
        eventListeners_.emplace(type, listener);
    }

    void post(const std::shared_ptr<IEvent>& event)
    {
        dispatcherThread_.process([this, event]() { this->dispatch(event); });
    }

    template<typename EventType, typename... Args>
    void post(Args&&... args)
    {
        auto event = std::make_shared<EventType>(std::forward<Args>(args)...);
        dispatcherThread_.process([this, event = std::move(event)]() { this->dispatch(event); });
    }

private:
    void dispatch(const std::shared_ptr<IEvent>& event)
    {
        const auto& [lisBegin, lisEnd] = eventListeners_.equal_range(event->type());
        for (auto listenerIt = lisBegin; listenerIt != lisEnd; listenerIt++)
        {
            listenerIt->second->onEvent(event);
        }
    }

    std::multimap<EventType, IEventListener*> eventListeners_;  // change to thread-safe multimap
    http::ThreadPool dispatcherThread_;
};
