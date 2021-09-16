#pragma once

#include <map>
#include <memory>
#include <optional>
#include <string>

#include "ClientInfo.h"
#include "SocketController.h"
#include "Url.h"
#include "ICallback.h"
#include "RequestWorker.h"
#include "HttpMethod.h"

namespace http
{
class HttpServer
{
public:
    HttpServer(std::string serverName);
    void registerCallback(HttpMethod method, const Url& url, const ICallback& callback);

    template<typename ReturnType, typename... FunctionArgs>
    void registerCallback(HttpMethod method, const Url& url, ReturnType(*function)(FunctionArgs...))
    {
        // class TemporaryCallback : public ICallback
        // {
        // } callback;
        // registerCallback(method, url, callback);
    }

private:
    std::string serverName_;
    std::unique_ptr<SocketController> socketController_;
    std::map<unsigned long long, RequestWorker> incomingRequests_;
};
}  // namespace http
