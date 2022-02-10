#include "SocketClientDataReceiver.h"

namespace http
{
namespace
{
    constexpr int DEFAULT_NUMBER_OF_THREADS = 3;
}
SocketClientDataReceiver::SocketClientDataReceiver(const std::shared_ptr<Socket>& socket)
    : socket_(socket), threads_(DEFAULT_NUMBER_OF_THREADS)
{
}

void SocketClientDataReceiver::asyncReceiveData(unsigned clientId)
{
    std::cout << "process async receive data" << std::endl;
    // async receive
    threads_.process([this, clientId]() {
        std::cout << "receiving data....." << std::endl;
        auto msg = socket_->receiveData(clientId); // request
        std::cout << "receiving data finished!!" << std::endl;
        if(!msg)
        {
            std::cout << "Error. Client removed before processing the connection." << std::endl;
            return;
        }
        std::cout << "Otrzymana wiadomosc ma dlugosc: " << msg->size() << std::endl;
        this->publish(ReceivedClientData{clientId, std::move(*msg)});
        //callback msg
        // send response
        //socket_->releaseClient(clientId); // if connection is not "keep-alive"
    });
    // do sth with data
}

void SocketClientDataReceiver::asyncSendData(unsigned clientId, const std::string& data)
{
    std::cout << "process async send data" << std::endl;
    // async receive
    threads_.process([this, clientId, data]() {
        std::cout << "Sending back data of length: " << data.size() << "\n" << data << std::endl;
        socket_->sendData(clientId, data); // request

        //socket_->releaseClient(clientId); // if connection is not "keep-alive"
    });
    // do sth with data
}

}  // namespace http