#pragma once

#include <memory>
#include <map>
#include <deque>
#include <optional>

#include "IEvent.hpp"
#include "IEventListener.hpp"
#include "ThreadPool.h"
#include "SafeQueue/SafeQueue.h"


struct EventDispatcher
{
public:
    EventDispatcher() : isRunning_{true}, workers_(NUM_OF_THREADS), dispatcherThread_(1)
    {
        dispatcherThread_.process([this](){ processEvents(); });
    }

    void listen(IEventListener* listener, EventType type)
    {
        eventListeners_.emplace(type, listener);
    }

    void post(const std::shared_ptr<IEvent>& event)
    {
        events_.enqueue(event);
    }

    template<typename EventType, typename... Args>
    void post(Args&&... args)
    {
        events_.enqueue(std::make_shared<EventType>(std::forward<Args>(args)...));
    }

    void dispatch(const std::shared_ptr<IEvent>& event)
    {
        const auto& [lisBegin, lisEnd] = eventListeners_.equal_range(event->type());
        for(auto listenerIt = lisBegin; listenerIt != lisEnd; listenerIt++)
        {
            listenerIt->second->onEvent(event);
        }
    }

    void processEvents()
    {
        while(isRunning_)
        {
            while(!events_.empty())
            {
                auto evOpt = events_.dequeue();
                if(evOpt)
                {
                    dispatch(*evOpt); // dispatch should be called on threads
                }
            }
        }
    }

private:
    static constexpr unsigned NUM_OF_THREADS = 2;
    std::multimap<EventType, IEventListener*> eventListeners_;
    http::ThreadPool workers_;
    http::ThreadPool dispatcherThread_;
    collections::SafeQueue<std::shared_ptr<IEvent>> events_;
    bool isRunning_;
};
