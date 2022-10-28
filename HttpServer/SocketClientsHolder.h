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
    std::shared_ptr<ConnectedSocket> getClient(unsigned clientId);
    unsigned getOrAddSocketClient(const SOCKET& client);
    std::vector<unsigned> getAllClientIds() const;
    std::shared_ptr<ConnectedSocket> operator[](unsigned clientId);
    std::optional<unsigned> getClientId(const ConnectedSocket& socket) const;

    void removeClient(unsigned clientId);

private:
    unsigned chooseNextId();
    std::unordered_map<unsigned, std::shared_ptr<ConnectedSocket>> clients_;
    unsigned clientId_ = 0;
    mutable std::mutex mutex_;
};
}
