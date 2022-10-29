#include "SocketClientsHolder.h"

#include <algorithm>
#include <functional>
#include <iostream>

#include <WinSock2.h>

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

std::shared_ptr<ConnectedSocket> SocketClientsHolder::getClient(unsigned clientId)
{
    std::lock_guard lock(mutex_);
    auto clientIter = clients_.find(clientId);
    if (clientIter != clients_.end())
    {
        return clientIter->second;
    }
    throw std::logic_error("No client with id " + std::to_string(clientId));
}

unsigned SocketClientsHolder::getOrAddSocketClient(const SOCKET& client)
{
    std::lock_guard lock(mutex_);
    const auto& clientIt = std::find_if(clients_.begin(), clients_.end(), [&client](const auto& idSocket) {
        return client == idSocket.second->getSocket();
    });
    if (clientIt != clients_.end())
    {
        std::cout << "This socket was already used, client known" << std::endl;
        return clientIt->first;
    }

    auto clientId = chooseNextId();
    clients_.emplace(clientId, std::make_shared<ConnectedSocket>(clientId, client));
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

std::shared_ptr<ConnectedSocket> SocketClientsHolder::operator[](unsigned clientId)
{
    std::lock_guard lock(mutex_);
    return clients_.at(clientId);
}

std::optional<unsigned> SocketClientsHolder::getClientId(const ConnectedSocket& client) const
{
    std::lock_guard lock(mutex_);
    const auto& clientIt = std::find_if(
        clients_.begin(), clients_.end(), [&client](const auto& idSocket) { return client == *idSocket.second; });
    if (clientIt != clients_.end())
    {
        return clientIt->first;
    }
    return {};
}

void SocketClientsHolder::removeClient(unsigned clientId)
{
    std::lock_guard lock(mutex_);
    clients_.erase(clientId);
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