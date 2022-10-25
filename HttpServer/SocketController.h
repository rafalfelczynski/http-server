#pragma once

#include <thread>

#include "Observer/IPublisher.h"
#include "Observer/ISubscriber.h"
#include "eventsystem/ClientConnectedEvent.hpp"
#include "eventsystem/ClientDataReceivedEvent.hpp"
#include "eventsystem/EventSystem.hpp"
#include "eventsystem/IEventListener.hpp"

#include "Socket.h"
#include "SocketClientDataReceiver.h"
#include "ThreadPool.h"

#include "HttpResponse.h"

#include "SocketClientsHolder.h"

namespace http
{
class ConnectionHandler
{
public:
    struct IListener
    {
        virtual ~IListener() = default;
        virtual void onClientConnected(unsigned clientId) = 0;
    };

    ConnectionHandler(const std::shared_ptr<ListeningSocket>& socket);
    ~ConnectionHandler();
    void setUp();
    void join();
    void setListener(IListener* listener);
    void removeListener();

private:
    std::shared_ptr<ListeningSocket> socket_;
    WorkerThread worker_;
    bool isListening_;
    IListener* listener_ = nullptr;
};

struct IHttpRequestListener
{
    virtual ~IHttpRequestListener() = default;
    virtual void onHttpRequest(const HttpRequest& req) = 0;
};

using ClientId = unsigned;

// change name, it should not be controller
class SocketController : private ConnectionHandler::IListener,
                         private SocketClientDataController::IListener
{
public:
    SocketController(std::string hostNameOrAddress, std::string serviceNameOrPort = "http");
    void join();
    void sendResponse(RequestId clientId, const HttpResponse& msg); // shouldn't be async listener rather than a method? Receive HttpResponse from server and parse it to string

    void addHttpListener(IHttpRequestListener* listener);

private:
    void onClientConnected(unsigned clientId) override;
    void onRequestReceived(const HttpRequest& clientData) override;
    void checkForNewClientConnections();

    std::shared_ptr<SocketClientsHolder> clientsHolder_;
    std::shared_ptr<ListeningSocket> socketInUse_;
    ConnectionHandler connectionHandler_;
    SocketClientDataController clientController_;

    std::vector<IHttpRequestListener*> httpReqListeners_;

    std::unordered_map<RequestId, ClientId> requestToClientMap_;
};
}  // namespace http
