#include "SocketClientsHolder.h"

#include <WinSock2.h>
#include <functional>
#include <algorithm>
#include <iostream>

namespace http
{
namespace
{
constexpr int DEFUALT_NUM_OF_CLIENTS = 1000;
}
SocketClientsHolder::SocketClientsHolder()
{
    clients_.reserve(DEFUALT_NUM_OF_CLIENTS);
}

std::optional<ConnectedSocket> SocketClientsHolder::getClient(unsigned clientId) const
{
    std::lock_guard lock(mutex_);
    auto clientIter = clients_.find(clientId);
    if (clientIter != clients_.end())
    {
        return {clientIter->second};
    }
    return {};
}

unsigned SocketClientsHolder::getOrAddSocketClient(const SOCKET& client)
{
    std::lock_guard lock(mutex_);
    const auto& clientIt = std::find_if(clients_.begin(), clients_.end(), [&client](const auto& idSocket){return client == idSocket.second.getSocket();});
    if(clientIt != clients_.end())
    {
        std::cout << "This socket was already used, client known" << std::endl;
        return clientIt->first;
    }

    auto clientId = chooseNextId();
    clients_.emplace(clientId, ConnectedSocket(clientId, client));
    return clientId;
}

std::vector<unsigned> SocketClientsHolder::getAllClientIds() const
{
    std::lock_guard lock(mutex_);
    std::vector<unsigned> clientIds;
    clientIds.reserve(clients_.size());
    for (const auto& client : clients_)
    {
        clientIds.push_back(client.first);
    }
    return clientIds;
}

ConnectedSocket SocketClientsHolder::operator[](unsigned clientId) const
{
    std::lock_guard lock(mutex_);
    return clients_.at(clientId);
}

std::optional<unsigned> SocketClientsHolder::getClientId(const ConnectedSocket& client) const
{
    std::lock_guard lock(mutex_);
    const auto& clientIt = std::find_if(clients_.begin(), clients_.end(), [&client](const auto& idSocket){return client == idSocket.second;});
    if(clientIt != clients_.end())
    {
        return clientIt->first;
    }
    return {};
}

unsigned SocketClientsHolder::chooseNextId()
{
    while (clients_.count(clientId_) != 0)
    {
        clientId_++;
    }
    return clientId_;
}
}  // namespace http