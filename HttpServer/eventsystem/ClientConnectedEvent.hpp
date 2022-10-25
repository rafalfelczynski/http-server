#pragma once
#include "IEvent.hpp"

class ClientConnectedEvent : public IEvent
{
public:
    static constexpr auto EVENT_TYPE = "ClientConnectedEvent";
    ClientConnectedEvent(unsigned cliId) : clientId{cliId}
    {}

    EventType type() const override { return EVENT_TYPE; }

    unsigned clientId;
};
