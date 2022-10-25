#pragma once

#include <optional>
#include <unordered_map>
#include <vector>
#include <mutex>

#include "ConnectedSocket.h"

namespace http
{
class SocketClientsHolder
{
public:
    SocketClientsHolder();
    std::optional<ConnectedSocket> getClient(unsigned clientId) const;
    unsigned getOrAddSocketClient(const SOCKET& client);
    std::vector<unsigned> getAllClientIds() const;
    ConnectedSocket operator[](unsigned clientId) const;
    std::optional<unsigned> getClientId(const ConnectedSocket& socket) const;

private:
    unsigned chooseNextId();
    std::unordered_map<unsigned, ConnectedSocket> clients_;
    unsigned clientId_ = 0;
    mutable std::mutex mutex_;
};
}
