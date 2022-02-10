#pragma once

#include <memory>

#include "Socket.h"
#include "ThreadPool.h"
#include "HttpRequest.h"
#include "Observer/IPublisher.h"

namespace http
{
struct ReceivedClientData
{
    const unsigned clientId;
    const std::string data;
};

class SocketClientDataReceiver : public observer::IPublisher<ReceivedClientData>
{
public:
    SocketClientDataReceiver(const std::shared_ptr<Socket>& socket);
    void asyncReceiveData(unsigned clientId); // should be async
    void asyncSendData(unsigned clientId, const std::string& data);

private:
    std::shared_ptr<Socket> socket_;
    ThreadPool threads_;
};
}  // namespace http
