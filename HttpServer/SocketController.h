#pragma once

#include <thread>

#include "Socket.h"
#include "ThreadPool.h"
#include "ConnectionServant.h"

namespace http
{
class SocketController
{
public:
    SocketController(const std::string& hostNameOrAddress, const std::string& serviceNameOrPort="http");

private:
    void waitForNewMessage();
    void checkForNewClientConnections();
    std::shared_ptr<Socket> &socketInUse_;
    ThreadPool threads_;
    std::unique_ptr<std::thread> blockingWorker_;
    std::unique_ptr<std::thread> nonBlockingWorker_;
};
}  // namespace http
