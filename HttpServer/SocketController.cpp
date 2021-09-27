#include "SocketController.h"

namespace http
{
SocketController::SocketController(const std::string& hostNameOrAddress, const std::string& serviceNameOrPort)
    : socketInUse_(std::make_shared<Socket>(hostNameOrAddress, serviceNameOrPort))
    , threads_(100)
    , isListening_(true)
{
    socketInUse_->bind();
    if (socketInUse_->isBound())
    {
        waitForNewMessage();
    }
}

void SocketController::waitForNewMessage()
{
    connectionListener_.acceptJob([this]() {
        while (isListening_)
        {
            auto clientId = socketInUse_->waitForClientToConnect();
            threads_.process([this, clientId]() {
                if (clientId.has_value())
                {
                    ConnectionServant servant{*clientId, socketInUse_};
                    servant.processClient();
                }
            });
        }
    });
}

void SocketController::checkForNewClientConnections()
{
}
}  // namespace http
