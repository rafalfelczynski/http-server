#pragma once

#include <iostream>
#include <string>

using EventType = std::string;

struct BadEventCast : std::exception
{
    BadEventCast(std::string msg) : msg_(std::move(msg))
    {
    }

    const char* what() const override
    {
        return msg_.c_str();
    }

private:
    std::string msg_;
};

struct IEvent
{
    virtual ~IEvent() = default;
    virtual EventType type() const = 0;
    template<typename Event>
    Event* cast()
    {
        auto* eventPtr = dynamic_cast<Event*>(this);
        if (eventPtr == nullptr)
        {
            throw BadEventCast("Cannot cast event " + type() + " to derived type.");
        }
        return eventPtr;
    }

    template<typename Event>
    const Event* cast() const
    {
        const auto* eventPtr = dynamic_cast<const Event*>(this);
        if (eventPtr == nullptr)
        {
            throw std::bad_cast("Cannot cast event " + type() + " to derived type.");
        }
        return eventPtr;
    }
};
