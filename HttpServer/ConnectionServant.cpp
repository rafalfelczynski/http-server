#include "ConnectionServant.h"

namespace http
{
ConnectionServant::ConnectionServant(unsigned clientId, const std::shared_ptr<Socket>& socket)
    : clientId_(clientId)
    , socket_(socket)
{
}

void ConnectionServant::processClient()
{
    if (socket_)
    {
        auto msg = socket_->receiveData(clientId_);
        socket_->releaseClient(clientId_);
    }
}
}  // namespace http