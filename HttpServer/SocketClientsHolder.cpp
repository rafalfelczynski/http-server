#include "SocketClientsHolder.h"

#include <WinSock2.h>
#include <functional>
#include <algorithm>

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

std::optional<SOCKET> SocketClientsHolder::getClient(unsigned clientId) const
{
    auto clientIter = clients_.find(clientId);
    if (clientIter != clients_.end())
    {
        return {clientIter->second};
    }
    return {};
}

unsigned SocketClientsHolder::getOrAddSocketClient(const SOCKET& client)
{
    const auto& clientIt = std::find_if(clients_.begin(), clients_.end(), [&client](const auto& idSocket){return client == idSocket.second;});
    if(clientIt != clients_.end())
    {
        return clientIt->first;
    }

    auto clientId = chooseNextId();
    clients_.insert({clientId, client});
    return clientId;
}

std::vector<unsigned> SocketClientsHolder::getAllClientIds() const
{
    std::vector<unsigned> clientIds;
    clientIds.reserve(clients_.size());
    for (const auto& client : clients_)
    {
        clientIds.push_back(client.first);
    }
    return clientIds;
}

SOCKET SocketClientsHolder::operator[](unsigned clientId) const
{
    return clients_.at(clientId);
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