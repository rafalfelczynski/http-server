#include "SocketController.h"

#include <algorithm>
#include "ThirdParty/Timer.hpp"

namespace http
{
ConnectionHandler::ConnectionHandler(
    const std::string& hostNameOrAddress,
    const std::string& serviceNameOrPort,
    const std::shared_ptr<SocketClientsHolder>& clientsHolder)
    : clientsHolder_(clientsHolder)
    , listeningSocket_(std::make_shared<ListeningSocket>(hostNameOrAddress, serviceNameOrPort))
    , isListening_(false)
{
    listeningSocket_->setClientsHolder(clientsHolder_);
}

ConnectionHandler::~ConnectionHandler()
{
    removeListener();
    isListening_ = false;
    listeningSocket_->release();
    if (worker_)
    {
        std::cout << "worker joining" << std::endl;
        worker_->joinAll();
    }
    std::cout << "all workers joined" << std::endl;
}

void ConnectionHandler::start()
{
    if (worker_)
    {
        return;
    }
    listeningSocket_->bind();
    if (!listeningSocket_->isBound())
    {
        return;
    }
std::cout << "listener started" << std::endl;
    worker_ = std::make_unique<ThreadPool>(1);
    worker_->init();
    isListening_ = true;
    worker_->process([this]() {
        while (isListening_)
        {
            std::cout << "waiting for client to connect" << std::endl;
            auto clientId = listeningSocket_->waitForClientToConnect();
            if (clientId)
            {
                std::cout << "client connected with id: " << *clientId << std::endl;
                if (listener_)
                {
                    listener_->onClientConnected(*clientId);
                }
            }
        }
    });
}

void ConnectionHandler::stop()
{
    std::cout << "listener stopped" << std::endl;
    isListening_ = false;
    if (worker_)
    {
        worker_->joinAll();
    }
}

void ConnectionHandler::join()
{
    std::cout << "listener joined" << std::endl;
    if (worker_)
    {
        worker_->joinAll();
    }
}

void ConnectionHandler::setListener(IListener* listener)
{
    this->listener_ = listener;
}

void ConnectionHandler::removeListener()
{
    this->listener_ = nullptr;
}

SocketController::SocketController(const std::string& hostNameOrAddress, const std::string& serviceNameOrPort)
    : clientsHolder_(std::make_shared<SocketClientsHolder>())
    , connectionHandler_(hostNameOrAddress, serviceNameOrPort, clientsHolder_)
{
    connectionHandler_.setListener(this);
}

SocketController::~SocketController()
{
    httpReqListener_ = nullptr;
    std::cout << "socket controller destr" << std::endl;
}

void SocketController::start()
{
    if(!httpReqListener_)
    {
        std::cout << "[CRITICAL] HttpRequestListener not initialized!" << std::endl;
        return;
    }
    connectionHandler_.start();
}

void SocketController::join()
{
    connectionHandler_.join();
}

// void SocketController::sendResponse(RequestId requestId, const HttpResponse& msg)
// {
//     // parse http response to string to send via socket
//     auto clientId = requestToClientMap_[requestId];
//     auto client = clientsHolder_->getClient(clientId);
//     client->sendData(msg.msg);
//     requestToClientMap_.erase(requestId);
// }

void SocketController::setHttpListener(IHttpRequestListener* listener)
{
    httpReqListener_ = listener;
}

void SocketController::onClientConnected(unsigned clientId)
{
    auto client = clientsHolder_->getClient(clientId);
    client->setListener(this);
    client->start();
}

void SocketController::onConnectionClosed(unsigned clientId)
{
    // workaround
    timer::SingleShotTimer::call(100, [this, clientId](){
        clientsHolder_->removeClient(clientId);
    });
}

std::string SocketController::onSocketDataReceived(unsigned clientId, const std::string& data)
{
    auto request = clientDataParser_.parseData(clientId, data);
    requestToClientMap_.emplace(request.getId(), request.getSender());
    if(httpReqListener_)
    {
        auto response = httpReqListener_->onHttpRequest(request);
        return response.msg; // response.toString();
        // auto client = clientsHolder_->getClient(clientId);
        // client->sendData(response.msg);
        // requestToClientMap_.erase(request.getId()); // may not be needed
    }
    return "";
}
}  // namespace http
