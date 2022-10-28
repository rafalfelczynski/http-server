#pragma once

// If linux include:
#ifdef linux

#else
// if windows include:
#ifdef _WIN32
#define NOMINMAX
#include <WS2tcpip.h>
#include <WinSock2.h>
#endif

#endif

#include <optional>
#include <string>
#include <vector>

#include "SocketClientsHolder.h"

#include "ConnectedSocket.h"

namespace http
{
enum class SocketState
{
    Idle,
    Waiting,
    Finished
};

// Should rather be rather ListeningSocket class?
class ListeningSocket
{
public:
    ListeningSocket(const std::string& host, const std::string& service = "http", unsigned bufferSize = 2048);
    ~ListeningSocket();
    void bind();
    void release();
    void setInListeningState();
    std::optional<unsigned> waitForClientToConnect();
    bool isBound() const;

    void setClientsHolder(const std::shared_ptr<SocketClientsHolder>& clientsHolder)
    {
        clientsHolder_ = clientsHolder;
    }

private:
    bool initializeSocketLib();
    bool getSocketAddressInfo();
    bool createSocketListener();
    bool bindListener();

    std::string host_;
    std::string service_;
    const unsigned bufferSize_;
    bool bound_ = false;
    ADDRINFO* sockAddressInfo_ = nullptr;
    SOCKET listener_;
    std::shared_ptr<SocketClientsHolder> clientsHolder_;
    std::unordered_map<SOCKET, sockaddr_in> socketToClientAddr_;
    mutable std::mutex mutex;
    std::unordered_map<SOCKET, SocketState> socketToState_;
};
}  // namespace http
