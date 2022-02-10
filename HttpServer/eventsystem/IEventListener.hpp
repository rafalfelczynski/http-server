#pragma once

#include <memory>
#include "IEvent.hpp"

struct IEventListener
{
    virtual ~IEventListener() = default;
    virtual void onEvent(const std::shared_ptr<IEvent>& event) = 0;
};
