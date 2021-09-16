#include "SocketController.h"

namespace http
{
SocketController::SocketController(const std::string& hostNameOrAddress, const std::string& serviceNameOrPort)
: socketInUse_(hostNameOrAddress, serviceNameOrPort)
{
    socketInUse_.bind();
    if (socketBoundCorrectly())
    {
    }
}

void SocketController::waitForNewMessage()
{
    auto clientId = socketInUse_.waitForClientToConnect();
    if (clientId.has_value())
    {
        auto msg = socketInUse_.receiveData(*clientId);
    }
}

void SocketController::checkForNewClientConnections() {}
}  // namespace http
