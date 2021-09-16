#include "ConnectionServant.h"

namespace http
{
ConnectionServant::ConnectionServant(unsigned clientId, const std::shared_ptr<Socket>& socket)
: clientId_(clientId), socket_(socket)
{
}
}  // namespace http