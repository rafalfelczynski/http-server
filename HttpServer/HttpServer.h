﻿#pragma once

#include <map>
#include <memory>
#include <optional>
#include <string>

#include "ClientInfo.h"
#include "HttpMethod.h"
#include "ICallback.h"
#include "RequestWorker.h"
#include "SocketController.h"
#include "Url.h"

namespace http
{
struct Endpoint
{
    Url url;
    HttpMethod method;
    Endpoint(Url url, HttpMethod method)
        : url(std::move(url))
        , method(method)
    {
    }
    bool operator==(const Endpoint&) const = default;
};
}  // namespace http

namespace std
{
template<>
struct hash<http::Endpoint>
{
    size_t operator()(const http::Endpoint& endpoint) const
    {
        // TODO
        return 0;
    }
};
}  // namespace std

namespace http
{
class HttpServer : private IHttpRequestListener
{
using CallbackFcn = std::function<std::string(const HttpRequest&)>;
public:
    HttpServer(std::string serverName);
    ~HttpServer();
    void registerCallback(HttpMethod method, const Url& url, std::unique_ptr<ICallback> callback);
    void registerCallback(HttpMethod method, const Url& url, CallbackFcn function);

    void run();

private:
    HttpResponse onHttpRequest(const HttpRequest& req) override;
    std::unique_ptr<SocketController> socketController_;
    std::unordered_map<Endpoint, std::unique_ptr<ICallback>> callbacks_;
};
}  // namespace http