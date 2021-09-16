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
    std::optional<SOCKET> getClient(unsigned clientId);
    unsigned addNewClient(const SOCKET& client);
    std::vector<unsigned> getAllClientIds();
    SOCKET operator[](unsigned clientId);

private:
    unsigned chooseNextId();
    std::unordered_map<unsigned, SOCKET> clients_;
    unsigned clientId_ = 0;
};
}
