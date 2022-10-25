#pragma once

#include <memory>

#include "Observer/IPublisher.h"

#include "HttpRequest.h"
#include "Socket.h"
#include "ThreadPool.h"

struct ReceivedClientData
{
    unsigned clientId;
    std::string data;
};

namespace http
{

struct ClientDataParser
{
    virtual ~ClientDataParser() = default;
    HttpRequest parseData(const ReceivedClientData& clientData) // maybe move, not const ref
    {
        static RequestId requestId = 0;
        // builder needed
        auto req = HttpRequest();
        req.id_ = requestId++;
        req.sender_ = clientData.clientId;
        req.content = clientData.data;
        return req;
    }
};

// Should have another thread to parseHttpRequests
class SocketClientDataController
{
public:
    struct IListener
    {
        virtual ~IListener() = default;
        virtual void onRequestReceived(const HttpRequest& request) = 0;
    };

    SocketClientDataController(const std::shared_ptr<ListeningSocket>& socket, const std::shared_ptr<SocketClientsHolder>& clientsHolder);
    ~SocketClientDataController();
    void asyncReceiveData(unsigned clientId);
    void asyncSendData(unsigned clientId, const std::string& data);
    void setListener(IListener* listener);
    void removeListener();

private:
    std::shared_ptr<ListeningSocket> socket_;
    ThreadPool dataReceiverWorkers_;
    IListener* listener_ = nullptr;
    ThreadPool httpDataParserWorkers_;
    ClientDataParser clientDataParser_;
    std::shared_ptr<SocketClientsHolder> clientsHolder_;
};
}  // namespace http
