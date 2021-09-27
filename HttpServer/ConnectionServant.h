#pragma once

#include <memory>

#include "Socket.h"

namespace http
{
class ConnectionServant
{
public:
    ConnectionServant(unsigned clientId, const std::shared_ptr<Socket>& socket);
    void processClient();

private:
    unsigned clientId_;
    std::shared_ptr<Socket> socket_;
};
}  // namespace http
