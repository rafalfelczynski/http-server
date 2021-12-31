#include "SocketClientDataReceiver.h"

namespace http
{
namespace
{
    constexpr int DEFAULT_NUMBER_OF_THREADS = 100;
}
SocketClientDataReceiver::SocketClientDataReceiver(const std::shared_ptr<Socket>& socket)
    : socket_(socket), threads_(DEFAULT_NUMBER_OF_THREADS)
{
}

void SocketClientDataReceiver::asyncReceiveData(unsigned clientId)
{
    std::cout << "process" << std::endl;
    // async receive
    threads_.process([this, clientId]() {
        auto msg = socket_->receiveData(clientId); // request
        if(!msg)
        {
            std::cout << "Error. Client removed before processing the connection." << std::endl;
            return;
        }
        this->publish(ReceivedClientData{clientId, std::move(*msg)});
        //callback msg
        // send response
        //socket_->releaseClient(clientId); // if connection is not "keep-alive"
    });
    // do sth with data
}
}  // namespace http