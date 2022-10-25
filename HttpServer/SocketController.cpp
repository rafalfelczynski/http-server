#include "SocketController.h"

#include <algorithm>

namespace http
{
ConnectionHandler::ConnectionHandler(const std::shared_ptr<ListeningSocket>& socket)
    : socket_(socket)
    //, connectionListener_{1}
    , isListening_(true)
{
}

ConnectionHandler::~ConnectionHandler()
{
    removeListener();
    isListening_ = false;
    worker_.join();
}

void ConnectionHandler::setUp()
{
    worker_.acceptJob([this]() {
        while (isListening_)
        {
            std::cout << "waiting for client to connect" << std::endl;
            auto clientId = socket_->waitForClientToConnect();
            if (clientId)
            {
                std::cout << "client connected with id: " << *clientId << std::endl;
                if (listener_)
                {
                    listener_->onClientConnected(*clientId);
                }
            }
            else
            {
                std::cout << "error client without id: " << std::endl;
                // error occurred
            }
        }
    });
}

void ConnectionHandler::join()
{
    worker_.join();
}

void ConnectionHandler::setListener(IListener* listener)
{
    this->listener_ = listener;
}

void ConnectionHandler::removeListener()
{
    this->listener_ = nullptr;
}

SocketController::SocketController(std::string hostNameOrAddress, std::string serviceNameOrPort)
    : clientsHolder_(std::make_shared<SocketClientsHolder>())
    , socketInUse_(std::make_shared<ListeningSocket>(
          std::move(hostNameOrAddress), std::move(serviceNameOrPort)))
    , connectionHandler_(socketInUse_)
    , clientController_(socketInUse_, clientsHolder_)
{
    socketInUse_->setClientsHolder(clientsHolder_);
    clientController_.setListener(this);
    socketInUse_->bind();
    if (socketInUse_->isBound())
    {
        connectionHandler_.setListener(this);
        connectionHandler_.setUp();
    }
}

void SocketController::join()
{
    connectionHandler_.join();
}

void SocketController::sendResponse(RequestId requestId, const HttpResponse& msg)
{
    // parse http response to string to send via socket
    clientController_.asyncSendData(requestToClientMap_[requestId], msg.msg);
    requestToClientMap_.erase(requestId);
}

void SocketController::addHttpListener(IHttpRequestListener* listener)
{
    if (listener)
    {
        if (std::find(httpReqListeners_.begin(), httpReqListeners_.end(), listener) == httpReqListeners_.end())
        {
            httpReqListeners_.push_back(listener);
        }
    }
}

void SocketController::onClientConnected(unsigned clientId)
{
    clientController_.asyncReceiveData(clientId);
}

void SocketController::onRequestReceived(const HttpRequest& request)
{
    requestToClientMap_.emplace(request.getId(), request.getSender());
    for (auto* lis : httpReqListeners_)
    {
        lis->onHttpRequest(request);
    }
}

void SocketController::checkForNewClientConnections()
{
}
}  // namespace http
