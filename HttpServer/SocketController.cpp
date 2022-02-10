#include "SocketController.h"
#include <algorithm>

namespace http
{
ConnectionListener::ConnectionListener(const std::shared_ptr<Socket>& socket)
    : socket_(socket)
    , connectionListener_{1}
    , isListening_(true)
{
    std::cout << "connection listener created" << std::endl;
}

void ConnectionListener::setUp()
{
    connectionListener_.process([this]() {
        while (isListening_)
        {
            std::cout << "waiting for client to connect" << std::endl;
            auto clientId = socket_->waitForClientToConnect();
            std::cout << "client connected" << std::endl;
            if (clientId.has_value())
            {
                std::cout << "client with id: " << *clientId << std::endl;
                this->publish(*clientId);
            }
            else
            {
                std::cout << "error client without id: " << *clientId << std::endl;
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

void SocketController::sendBack(unsigned clientId, const std::string& msg) 
{
    std::cout << "sending back" << std::endl;
    clientController_.asyncSendData(clientId, msg);
}

void SocketController::onPublisherNotification(const unsigned& clientId) // called when new client has been connected
{
    std::cout << "client connected. receive process" << std::endl;
    // static int i=0;
    // auto iStr = std::to_string(i);

    // auto msg = "HTTP/1.1 200 OK\r\n"
    //             "Cache-Control: no-cache, private\r\n"
    //             "Content-Type: text/plain\r\n"
    //             "Content-Length: 8\r\n"
    //             "\r\n"
    //             "Hello" + std::string(3 - std::min(size_t(3), iStr.size()), '0') + iStr;
    // i++;
    // clientController_.asyncSendData(clientId, msg);
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
