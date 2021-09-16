#include "HttpServer.h"

#include <utility>

namespace http
{
HttpServer::HttpServer(std::string serverName, const std::optional<unsigned>& socketsLimit)
: serverName_(std::move(serverName)), socketsLimit_(socketsLimit)
{
}

}  // namespace http
