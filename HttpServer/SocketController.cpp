#include "SocketController.h"

namespace http
{
ConnectionListener::ConnectionListener(const std::shared_ptr<Socket>& socket)
    : socket_(socket)
    , connectionListener_{1}
    , isListening_(true)
{
}

void ConnectionListener::setUp()
{
    connectionListener_.process([this]() {
        while (isListening_)
        {
            std::cout << "waiting" << std::endl;
            auto clientId = socket_->waitForClientToConnect();
            std::cout << "client connected" << std::endl;
            if (clientId.has_value())
            {
                this->publish(*clientId);
            }
            else
            {
                // error occurred
            }
        }
    });
}

void ConnectionListener::join()
{
    connectionListener_.joinAll();
}

SocketController::SocketController(std::string hostNameOrAddress, std::string serviceNameOrPort)
    : socketInUse_(std::make_shared<Socket>(std::move(hostNameOrAddress), std::move(serviceNameOrPort)))
    , connectionListener_(socketInUse_)
    , clientController_(socketInUse_)
{
    connectionListener_.addObserver(this);
    clientController_.addObserver(this);
    socketInUse_->bind();
    if (socketInUse_->isBound())
    {
        connectionListener_.setUp();
    }
}

void SocketController::join() 
{
    connectionListener_.join();
}

void SocketController::onPublisherNotification(const unsigned& clientId) // called when new client has been connected
{
    clientController_.asyncReceiveData(clientId);
}

void SocketController::onPublisherNotification(const ReceivedClientData& clientData) // called when client sent their mesage
{
    std::cout << "msg received" << std::endl;
    this->publish(clientData);
}

void SocketController::checkForNewClientConnections()
{
}
}  // namespace http
