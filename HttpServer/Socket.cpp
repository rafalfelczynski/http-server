#include "Socket.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>

// Need to link with Ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

#include <utility>

namespace
{
ADDRINFOA createDefaultConnectionSettings()
{
    ADDRINFOA connectionSettings{};
    connectionSettings.ai_family = AF_INET;
    connectionSettings.ai_socktype = SOCK_STREAM;
    connectionSettings.ai_protocol = IPPROTO_TCP;
    connectionSettings.ai_flags = AI_PASSIVE;
    return connectionSettings;
}
}  // namespace

namespace http
{
Socket::Socket(std::string hostNameOrAddress, std::string serviceNameOrPort, const unsigned bufferSize)
: hostNameOrAddress_(std::move(hostNameOrAddress))
, serviceNameOrPort_(std::move(serviceNameOrPort))
, bufferSize_(bufferSize)
{
}

Socket::~Socket() {}

void Socket::bind()
{
    if (!initializeSocketLib())
    {
        return;
    }

    if (!getSocketAddressInfo())
    {
        return;
    }

    if (!createSocketListener())
    {
        return;
    }

    if (!bindListener())
    {
        return;
    }
    setInListeningState();
    bound_ = true;
    std::cout << "Socket bound correctly!" << std::endl;
}

void Socket::release()
{
    for (const auto& clientId : clientsHolder_.getAllClientIds())
    {
        shutdown(clientsHolder_[clientId], SD_BOTH);
        closesocket(clientsHolder_[clientId]);
    }
    shutdown(listener_, SD_BOTH);
    freeaddrinfo(addressInfo_);
    closesocket(listener_);
    WSACleanup();
    bound_ = false;
}

void Socket::setInListeningState()
{
    auto listenResult = listen(listener_, SOMAXCONN);
    if (listenResult == SOCKET_ERROR)
    {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(listener_);
        WSACleanup();
    }
}

std::optional<unsigned> Socket::waitForClientToConnect()
{
    auto client = accept(listener_, &clientInfo_, 0);
    if (client == INVALID_SOCKET)
    {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(client);
        return {};
    }
    return {clientsHolder_.addNewClient(client)};
}

void Socket::asyncListenClientToConnect() {}

std::optional<std::vector<char>> Socket::receiveData(unsigned clientId)
{
    auto client = clientsHolder_.getClient(clientId);
    if (!client.has_value())
    {
        std::cout << "There is not client with id " << clientId << std::endl;
        return {};
    }
    return {processMessage(*client)};
}

void Socket::sendData(unsigned clientId, const std::string& msg)
{
    auto client = clientsHolder_.getClient(clientId);
    if (!client.has_value())
    {
        std::cout << "There is no client with id " << clientId << " The data won't be sent" << std::endl;
        return;
    }
    send(*client, msg.data(), msg.length(), (int)0);
    shutdown(*client, SD_SEND);
}

bool Socket::initializeSocketLib()
{
    // Initialize Winsock
    WSADATA socketInfo;
    auto startupResult = WSAStartup(MAKEWORD(2, 2), &socketInfo);
    if (startupResult != 0)
    {
        printf("WSAStartup failed! with error: %d\n", WSAGetLastError());
        return false;
    }
    return true;
}

bool Socket::getSocketAddressInfo()
{
    auto connectionSettings = createDefaultConnectionSettings();
    auto addrinfoResult =
        getaddrinfo(hostNameOrAddress_.c_str(), serviceNameOrPort_.c_str(), &connectionSettings, &addressInfo_);
    if (addrinfoResult != 0)
    {
        printf("getaddrinfo failed with error: %d\n", addrinfoResult);
        WSACleanup();
        return false;
    }
    return true;
}

bool Socket::createSocketListener()
{
    listener_ = socket(addressInfo_->ai_family, addressInfo_->ai_socktype, addressInfo_->ai_protocol);
    if (listener_ == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(addressInfo_);
        WSACleanup();
        return false;
    }
    return true;
}

bool Socket::bindListener()
{
    auto bindResult = ::bind(listener_, addressInfo_->ai_addr, addressInfo_->ai_addrlen);
    if (bindResult == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(addressInfo_);
        closesocket(listener_);
        WSACleanup();
        return false;
    }
    return true;
}

int Socket::processPartOfMsg(const SOCKET& client, std::vector<char>& receivedMsg) const
{
    std::vector<char> msgPart;
    msgPart.resize(bufferSize_);
    auto numOfBytes = recv(client, msgPart.data(), bufferSize_, 0);
    if (numOfBytes == SOCKET_ERROR)
    {
        printf("recv failed with error: %d\n", WSAGetLastError());
        return -1;
    }
    else if (numOfBytes > 0)
    {
        receivedMsg.insert(receivedMsg.end(), msgPart.data(), msgPart.data() + numOfBytes);
    }
    return numOfBytes;
}

std::vector<char> Socket::processMessage(const SOCKET& client) const
{
    // Receive until the peer shuts down the connection
    std::vector<char> receivedMsg;
    receivedMsg.reserve(bufferSize_);
    auto numOfBytes = 0;
    do
    {
        numOfBytes = processPartOfMsg(client, receivedMsg);
    } while (numOfBytes > 0);
    receivedMsg.shrink_to_fit();
    return receivedMsg;
}

}  // namespace http
