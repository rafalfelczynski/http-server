#include "ConnectedSocket.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <utility>

namespace
{
constexpr unsigned SOCKET_BUFFER_SIZE = 4096;
}  // namespace

namespace http
{
ConnectedSocket::ConnectedSocket(unsigned clientId, SOCKET socket)
    : clientId_(clientId)
    , client_(socket)
    , readingFinished_(false)
    , closed_(false)
{
}

ConnectedSocket::~ConnectedSocket()
{
    removeListener();
    close();
    std::cout << "Connectedsocket JoinAll start client " << clientId_ << std::endl;
    workers_->joinAll();
    std::cout << "Connectedsocket JoinAll end client " << clientId_ << std::endl;
}

void ConnectedSocket::start()
{
    if(closed_)
    {
        return;
    }
    workers_ = std::make_unique<ThreadPool>(1);
    workers_->init();
    workers_->process([this]() { run(); });
}

void ConnectedSocket::run()
{
    std::cout << "receiving data..... client " << clientId_ << std::endl;
    while (!isClosed())
    {
        auto msg = receiveData();
        std::cout << "client: " << clientId_ << " receiving data finished!! Message size: " << msg.size() << " is finished: " << isFinished() << std::endl;
        if (!msg.empty())
        {
            if (listener_)
            {
                auto responseToSend = listener_->onSocketDataReceived(clientId_, msg);
                if(!responseToSend.empty())
                {
                    sendData(responseToSend);
                }
            }
        }
    }
    std::cout << "Connected socket was closed" << std::endl;
}

std::string ConnectedSocket::receiveData()
{
    if(closed_)
    {
        return "";
    }
    // Receive until the peer shuts down the connection
    std::string receivedMsg;
    receivedMsg.reserve(SOCKET_BUFFER_SIZE);
    int numOfBytes = 0;
    printf("Receive called client: %d\n", clientId_);
    waitForDataToArrive();
    while (isReadyToRead())
    {
        printf("Receive part will be called for client: %d\n", clientId_);
        numOfBytes = receivePartOfMsg(receivedMsg);
        updateSocketStateFlags(numOfBytes);
    }
    updateSocketStateFlags(numOfBytes);
    std::cout << "finished receiving data from client: " << clientId_ << " Data size: " << receivedMsg.size()
              << std::endl;
    return receivedMsg;
}

void ConnectedSocket::sendData(const std::string& msg)
{
    if(closed_)
    {
        return;
    }
    send(client_, msg.data(), static_cast<int>(msg.size()), 0);
    //workers_->process([this, msg]() { send(client_, msg.data(), static_cast<int>(msg.size()), 0); });
}

bool ConnectedSocket::operator==(const ConnectedSocket& oth) const
{
    return this->clientId_ == oth.clientId_ && this->client_ == oth.client_;
}

SOCKET ConnectedSocket::getSocket() const
{
    return client_;
}

bool ConnectedSocket::isClosed() const
    {
        return closed_;
    }

int ConnectedSocket::receivePartOfMsg(std::string& receivedMsg)
{
    std::string msgPart(SOCKET_BUFFER_SIZE, '\0');
    printf("Recv client: %d\n", clientId_);
    auto numOfBytes = recv(client_, msgPart.data(), static_cast<int>(msgPart.size()), 0);
    if (numOfBytes == SOCKET_ERROR)
    {
        printf("recv failed with error: %d on client %d\n", WSAGetLastError(), clientId_);
        return -1;
    }

    if (numOfBytes > 0)
    {
        receivedMsg.insert(receivedMsg.end(), msgPart.data(), msgPart.data() + numOfBytes);
    }
    printf("Receive part returned for client: %d with size: %d\n", clientId_, numOfBytes);
    return numOfBytes;
}

void ConnectedSocket::waitForDataToArrive()
{
    // fd_set clientSocketSet{};
    // clientSocketSet.fd_count = 1;
    // clientSocketSet.fd_array[0] = client_;
    // select(0, &clientSocketSet, nullptr, nullptr, nullptr);
    pollfd clientSocketSet{};
    clientSocketSet.fd = client_;
    clientSocketSet.events = POLLIN;
    WSAPoll(&clientSocketSet, 1, -1);
}

bool ConnectedSocket::isReadyToRead() const
{
    static constexpr short READ_FLAGS = POLLIN;
    pollfd clientSocketSet{};
    clientSocketSet.fd = client_;
    clientSocketSet.events = READ_FLAGS;
    auto numOfReadySockets = WSAPoll(&clientSocketSet, 1, 0);
    // fd_set clientSocketSet{};
    // clientSocketSet.fd_count = 1;
    // clientSocketSet.fd_array[0] = client_;
    // timeval timeout{0, 10};
    // auto numOfReadySockets = select(0, &clientSocketSet, nullptr, nullptr, &timeout);
    if (numOfReadySockets <= 0)
    {
        // timeout or error occured
        return false;
    }
    // select passed - socket is either ready or closed
    // previously read some bytes and select > 0 ---> means that it has more
    // previously read 0 bytes and select > 0 ----> means that socket was closed or it's the first read
    // previously read -1 bytes and select > 0 ----> means that socket has error
    return (clientSocketSet.revents & READ_FLAGS) && !closed_ && !readingFinished_;
}

void ConnectedSocket::updateSocketStateFlags(int numOfReadBytes)
{
    if(numOfReadBytes == 0)
    {
        // socket was closed when recv returned 0
        close();
        readingFinished_ = true;
    }
    else if (numOfReadBytes < 0)
    {
        // socket error
        readingFinished_ = true;
        std::cout << "Connected socket finished reading" << std::endl;
    }
}

void ConnectedSocket::close()
{
    if(closed_)
    {
        return;
    }
    std::cout << "Client " << clientId_ << " connection shut" << std::endl;
    readingFinished_ = true;
    closed_ = true;
    shutdown(client_, SD_BOTH);
    closesocket(client_);
    if(listener_)
    {
        listener_->onConnectionClosed(clientId_);
    }
}
}  // namespace http
