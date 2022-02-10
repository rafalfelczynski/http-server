#pragma once

#include <thread>

#include "Observer/IPublisher.h"
#include "Observer/ISubscriber.h"

#include "SocketClientDataReceiver.h"
#include "Socket.h"
#include "ThreadPool.h"

namespace http
{
class ConnectionListener : public observer::IPublisher<unsigned>
{
public:
    ConnectionListener(const std::shared_ptr<Socket>& socket);
    void setUp();
    void join();

private:
    std::shared_ptr<Socket> socket_;
    ThreadPool connectionListener_;
    bool isListening_;
};

class SocketController : private observer::ISubscriber<unsigned>, public observer::ISubscriber<ReceivedClientData>, public observer::IPublisher<ReceivedClientData>
{
public:
    SocketController(std::string hostNameOrAddress, std::string serviceNameOrPort = "http");
    void join();
    void sendBack(unsigned clientId, const std::string& msg);

private:
    void onPublisherNotification(const unsigned& clientId) override;
    void onPublisherNotification(const ReceivedClientData& clientData) override;
    void checkForNewClientConnections();
    std::shared_ptr<Socket> socketInUse_;
    ConnectionListener connectionListener_;
    SocketClientDataReceiver clientController_;
};
}  // namespace http
