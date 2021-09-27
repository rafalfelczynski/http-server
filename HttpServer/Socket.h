#pragma once

// If linux include:
#ifdef linux

#else
// if windows include:
#ifdef _WIN32
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
    Socket(std::string hostNameOrAddress, std::string serviceNameOrPort = "http", const unsigned bufferSize = 512);
    ~Socket();
    void bind();
    void release();
    void setInListeningState();
    std::optional<unsigned> waitForClientToConnect();
    void asyncListenClientToConnect();
    std::optional<std::vector<char>> receiveData(unsigned clientId);
    void sendData(unsigned clientId, const std::string& msg);
    bool isBound();
    void releaseClient(unsigned clientId);

private:
    bool initializeSocketLib();
    bool getSocketAddressInfo();
    bool createSocketListener();
    bool bindListener();
    int processPartOfMsg(const SOCKET& client, std::vector<char>& receivedMsg) const;
    std::vector<char> processMessage(const SOCKET& client) const;
    std::string hostNameOrAddress_;
    std::string serviceNameOrPort_;
    const unsigned bufferSize_;
    bool bound_ = false;
    ADDRINFO* addressInfo_ = nullptr;
    SOCKET listener_;
    SocketClientsHolder clientsHolder_;
    sockaddr clientInfo_;
};
}  // namespace http
