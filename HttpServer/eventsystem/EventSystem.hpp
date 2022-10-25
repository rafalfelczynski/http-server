#pragma once

#include <memory>
#include <deque>
#include "IEvent.hpp"
#include "EventDispatcher.hpp"

class EventSystem
{
public:
    static EventSystem& getInstance()
    {
        static EventSystem system;
        return system;
    }

    void listen(IEventListener* listener, EventType type)
    {
        eventDispatcher_.listen(listener, type);
    }

    void post(const std::shared_ptr<IEvent>& event)
    {
        eventDispatcher_.post(event);
    }

    template<typename EventType, typename... Args>
    void post(Args&&... args)
    {
        eventDispatcher_.post<EventType, Args...>(std::forward<Args>(args)...);
    }

private:
    EventSystem() = default;
    EventDispatcher eventDispatcher_;
};
