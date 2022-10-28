#pragma once

#include <thread>

#include "Observer/IPublisher.h"
#include "Observer/ISubscriber.h"
#include "eventsystem/ClientConnectedEvent.hpp"
#include "eventsystem/ClientDataReceivedEvent.hpp"
#include "eventsystem/EventSystem.hpp"
#include "eventsystem/IEventListener.hpp"

#include "HttpRequest.h"
#include "HttpResponse.h"
#include "Socket.h"
#include "SocketClientsHolder.h"
#include "ThreadPool.h"

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

    ConnectionHandler(
        const std::string& hostNameOrAddress,
        const std::string& serviceNameOrPort,
        const std::shared_ptr<SocketClientsHolder>& clientsHolder);
    ~ConnectionHandler();
    void start();
    void stop();
    void join(); // temporary for testing
    void setListener(IListener* listener);
    void removeListener();

private:
    std::shared_ptr<SocketClientsHolder> clientsHolder_;
    std::shared_ptr<ListeningSocket> listeningSocket_;
    std::unique_ptr<ThreadPool> worker_;
    bool isListening_;
    IListener* listener_ = nullptr;
};

struct IHttpRequestListener
{
    virtual ~IHttpRequestListener() = default;
    virtual HttpResponse onHttpRequest(const HttpRequest& req) = 0;
};

struct ClientDataParser
{
    virtual ~ClientDataParser() = default;
    HttpRequest parseData(unsigned clientId, const std::string& clientData)  // maybe move, not const ref
    {
        static RequestId requestId = 0;
        // builder needed
        auto req = HttpRequest();
        req.id_ = requestId++;
        req.sender_ = clientId;
        req.content = clientData;
        return req;
    }
};

using ClientId = unsigned;

// change name, it should not be controller
class SocketController : private ConnectionHandler::IListener, private ISocketDataListener
{
public:
    SocketController(const std::string& hostNameOrAddress, const std::string& serviceNameOrPort);
    ~SocketController();
    void start();
    void join();
    // void sendResponse(RequestId clientId, const HttpResponse& msg);
    void setHttpListener(IHttpRequestListener* listener);

private:
    void onClientConnected(unsigned clientId) override;
    void onConnectionClosed(unsigned clientId) override;
    std::string onSocketDataReceived(unsigned socketId, const std::string& data) override;

    std::shared_ptr<SocketClientsHolder> clientsHolder_;
    // std::shared_ptr<ListeningSocket> listeningSocket_;
    ConnectionHandler connectionHandler_;
    ClientDataParser clientDataParser_;

    IHttpRequestListener* httpReqListener_ = nullptr;

    std::unordered_map<RequestId, ClientId> requestToClientMap_;
};
}  // namespace http
