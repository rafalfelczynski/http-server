#pragma once

#include <optional>
#include <unordered_map>
#include <vector>

#include <WinSock2.h>

namespace http
{
class SocketClientsHolder
{
public:
    SocketClientsHolder();
    std::optional<SOCKET> getClient(unsigned clientId) const;
    unsigned getOrAddSocketClient(const SOCKET& client);
    std::vector<unsigned> getAllClientIds() const;
    SOCKET operator[](unsigned clientId) const;

private:
    unsigned chooseNextId();
    std::unordered_map<unsigned, SOCKET> clients_;
    unsigned clientId_ = 0;
};
}
