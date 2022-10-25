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
ListeningSocket::ListeningSocket(std::string host, std::string service, unsigned bufferSize)
    : host_(std::move(host))
    , service_(std::move(service))
    , bufferSize_(bufferSize)
{
}

ListeningSocket::~ListeningSocket()
{
}

void ListeningSocket::bind()
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

void ListeningSocket::release()
{
    for (const auto& clientId : clientsHolder_->getAllClientIds())
    {
        clientsHolder_->getClient(clientId)->close();
    }
    shutdown(listener_, SD_BOTH);
    freeaddrinfo(sockAddressInfo_);
    closesocket(listener_);
    WSACleanup();
    bound_ = false;
}

void ListeningSocket::setInListeningState()
{
    auto listenResult = listen(listener_, SOMAXCONN);
    if (listenResult == SOCKET_ERROR)
    {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(listener_);
        WSACleanup();
    }
}

std::optional<unsigned> ListeningSocket::waitForClientToConnect()
{
    sockaddr_in clientAddr{};
    clientAddr.sin_family = AF_INET;
    int clientAddrLength = sizeof(sockaddr_in);
    auto client = accept(listener_, (sockaddr*)&clientAddr, &clientAddrLength);
    if (client == INVALID_SOCKET)
    {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(client);
        return {};
    }
    std::string socketClientIp(16, '\0');
    if (!inet_ntop(AF_INET, &clientAddr.sin_addr, socketClientIp.data(), socketClientIp.size()))
    {
        printf("Failed to convert client to ip address: %d\n", WSAGetLastError());
        closesocket(client);
        return {};
    }
    std::lock_guard lock(mutex);
    socketToClientAddr_[client] = clientAddr;
    socketToState_[client] = SocketState::Idle;
    return {clientsHolder_->getOrAddSocketClient(client)};
}

void ListeningSocket::asyncListenClientToConnect()
{
}

// std::string ListeningSocket::receiveData(unsigned clientId)
// {
//     auto client = clientsHolder_.getClient(clientId);
//     if (!client)
//     {
//         std::cout << "Error. Client removed before processing the connection." << std::endl;
//         return "";
//     }
//     return receiveMessageFromClient(*client);
// }

// void ListeningSocket::sendData(unsigned clientId, const std::string& msg) const
// {
//     auto client = clientsHolder_.getClient(clientId);
//     if (!client)
//     {
//         std::cout << "There is no client with id " << clientId << " The data won't be sent" << std::endl;
//         return;
//     }
//     std::lock_guard lock(mutex);
//     sendto(*client, msg.data(), msg.size(), 0, (sockaddr*)&socketToClientAddr_.at(*client), sizeof(sockaddr));
// }

bool ListeningSocket::initializeSocketLib()
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

bool ListeningSocket::getSocketAddressInfo()
{
    auto connectionSettings = createDefaultConnectionSettings();
    auto addrinfoResult = getaddrinfo(host_.c_str(), service_.c_str(), &connectionSettings, &sockAddressInfo_);
    if (addrinfoResult != 0)
    {
        printf("getaddrinfo failed with error: %d\n", addrinfoResult);
        WSACleanup();
        return false;
    }
    return true;
}

bool ListeningSocket::createSocketListener()
{
    listener_ = socket(sockAddressInfo_->ai_family, sockAddressInfo_->ai_socktype, sockAddressInfo_->ai_protocol);
    if (listener_ == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(sockAddressInfo_);
        WSACleanup();
        return false;
    }
    // u_long mode = 1;  // 1 to enable non-blocking socket
    // ioctlsocket(listener_, FIONBIO, &mode);
    return true;
}

bool ListeningSocket::bindListener()
{
    auto bindResult = ::bind(listener_, sockAddressInfo_->ai_addr, static_cast<int>(sockAddressInfo_->ai_addrlen));
    if (bindResult == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(sockAddressInfo_);
        closesocket(listener_);
        WSACleanup();
        return false;
    }
    return true;
}

bool hasMoreDataToRead(const SOCKET& client, int lastNumOfBytes)
{
    fd_set clientSocketSet{};
    clientSocketSet.fd_count = 1;
    clientSocketSet.fd_array[0] = client;
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

void waitForDataToArrive(const SOCKET& client)
{
    fd_set clientSocketSet{};
    clientSocketSet.fd_count = 1;
    clientSocketSet.fd_array[0] = client;
    select(0, &clientSocketSet, nullptr, nullptr, nullptr);
}

// std::string ListeningSocket::receiveMessageFromClient(const SOCKET& client)
// {
//     // Receive until the peer shuts down the connection
//     std::string receivedMsg;
//     receivedMsg.reserve(bufferSize_);
//     int numOfBytes = 0;
//     printf("Receive called client: %d\n", *clientsHolder_.getClientId(client));
//     waitForDataToArrive(client);
//     do
//     {
//         printf("Receive part will be called for client: %d\n", *clientsHolder_.getClientId(client));
//         numOfBytes = receivePartOfMsg(client, receivedMsg);
//     } while (hasMoreDataToRead(client, numOfBytes));

//     if(numOfBytes <= 0)
//     {
//         // socket was closed or error
//         socketToState_[client] = SocketState::Finished;
//     }
//     std::cout << "finished receiving data from client: " << *clientsHolder_.getClientId(client)
//               << " Data size: " << receivedMsg.size() << std::endl;
//     return receivedMsg;
// }

// int ListeningSocket::receivePartOfMsg(const SOCKET& client, std::string& receivedMsg)
// {
//     std::string msgPart(bufferSize_, '\0');
//     printf("Recv client: %d\n", *clientsHolder_.getClientId(client));
//     auto numOfBytes = recv(client, msgPart.data(), bufferSize_, 0);
//     if (numOfBytes == SOCKET_ERROR)
//     {
//         printf("recv failed with error: %d on client %d\n", WSAGetLastError(), *clientsHolder_.getClientId(client));
//         return -1;
//     }

//     if (numOfBytes > 0)
//     {
//         receivedMsg.insert(receivedMsg.end(), msgPart.data(), msgPart.data() + numOfBytes);
//     }
//     printf("Receive part returned for client: %d with size: %d\n", *clientsHolder_.getClientId(client), numOfBytes);
//     return numOfBytes;
// }

bool ListeningSocket::isBound() const
{
    return bound_;
}
}  // namespace http
