#include "Socket.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
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
ListeningSocket::ListeningSocket(const std::string& host, const std::string& service, unsigned bufferSize)
    : host_(host)
    , service_(service)
    , bufferSize_(bufferSize)
{
}

ListeningSocket::~ListeningSocket()
{
    if(isBound())
    {
        release();
    }
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
}

void ListeningSocket::release()
{
    shutdown(listener_, SD_BOTH);
    freeaddrinfo(sockAddressInfo_);
    closesocket(listener_);
    //WSACleanup();
    bound_ = false;
    std::cout << "listener released" << std::endl;
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

bool ListeningSocket::isBound() const
{
    return bound_;
}
}  // namespace http
