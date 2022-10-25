#pragma once
#include "IEvent.hpp"

class ClientDataReceivedEvent : public IEvent
{
public:
    // static constexpr auto EVENT_TYPE = "ClientDataReceivedEvent";
    // ClientDataReceivedEvent(ReceivedClientData cliData, void* sender) : clientData{std::move(cliData)}, sender{sender}
    // {}

    // EventType type() const override { return EVENT_TYPE; }

    // ReceivedClientData clientData;
    // void* sender;
};
