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

namespace http
{
class Socket
{
public:
    Socket(std::string host, std::string service = "http", unsigned bufferSize = 2048);
    ~Socket();
    void bind();
    void release();
    void setInListeningState();
    std::optional<unsigned> waitForClientToConnect();
    void asyncListenClientToConnect();
    std::optional<std::string> receiveData(unsigned clientId) const;
    void sendData(unsigned clientId, const std::string& msg) const;
    bool isBound() const;
    void releaseClient(unsigned clientId) const;

private:
    bool initializeSocketLib();
    bool getSocketAddressInfo();
    bool createSocketListener();
    bool bindListener();
    int processPartOfMsg(const SOCKET& client, std::string& receivedMsg) const;
    std::string processMessage(const SOCKET& client) const;

    std::string host_;
    std::string service_;
    const unsigned bufferSize_;
    bool bound_ = false;
    ADDRINFO* addressInfo_ = nullptr;
    SOCKET listener_;
    SocketClientsHolder clientsHolder_;
    sockaddr clientInfo_;
};
}  // namespace http
