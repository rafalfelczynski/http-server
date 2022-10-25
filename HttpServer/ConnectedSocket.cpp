#include "ConnectedSocket.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>

// Need to link with Ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

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
    , finished_(false)
{
}

std::string ConnectedSocket::receiveData() const
{
    // Receive until the peer shuts down the connection
    std::string receivedMsg;
    receivedMsg.reserve(SOCKET_BUFFER_SIZE);
    int numOfBytes = 0;
    printf("Receive called client: %d\n", clientId_);
    waitForDataToArrive();
    do
    {
        printf("Receive part will be called for client: %d\n", clientId_);
        numOfBytes = receivePartOfMsg(receivedMsg);
    } while (hasMoreDataToRead(numOfBytes));

    if (numOfBytes <= 0)
    {
        // socket was closed or error
        finished_ = true;
        // socketToState_[client] = SocketState::Finished;
    }
    std::cout << "finished receiving data from client: " << clientId_ << " Data size: " << receivedMsg.size()
              << std::endl;
    return receivedMsg;
}

void ConnectedSocket::sendData(const std::string& msg) const
{
    send(client_, msg.data(), static_cast<int>(msg.size()), 0);
}

bool ConnectedSocket::operator==(const ConnectedSocket& oth) const
{
    return this->clientId_ == oth.clientId_ && this->client_ == oth.client_;
}

SOCKET ConnectedSocket::getSocket() const
{
    return client_;
}

int ConnectedSocket::receivePartOfMsg(std::string& receivedMsg) const
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

void ConnectedSocket::waitForDataToArrive() const
{
    fd_set clientSocketSet{};
    clientSocketSet.fd_count = 1;
    clientSocketSet.fd_array[0] = client_;
    select(0, &clientSocketSet, nullptr, nullptr, nullptr);
}

bool ConnectedSocket::hasMoreDataToRead(int lastNumOfBytes) const
{
    fd_set clientSocketSet{};
    clientSocketSet.fd_count = 1;
    clientSocketSet.fd_array[0] = client_;
    timeval timeout{0, 10};
    auto numOfReadySockets = select(0, &clientSocketSet, nullptr, nullptr, &timeout);
    if (numOfReadySockets == 0)
    {
        return false;
    }
    // previously read some bytes and select != 0 ---> means that it has more
    // previously read 0 bytes and select != ----> means that socket was closed
    // previously read -1 bytes and select != ----> means that socket has error
    return lastNumOfBytes > 0;
}

void ConnectedSocket::close()
{
    std::cout << "Client " << clientId_ << "connection shut" << std::endl;
    shutdown(client_, SD_BOTH);
    closesocket(client_);
}
}  // namespace http
