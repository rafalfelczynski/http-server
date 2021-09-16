#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <tuple>

#include "HttpMethod.h"
#include "ICallback.h"
#include "Url.h"

namespace http
{
using CallbacksMap = std::map<HttpMethod, std::unique_ptr<ICallback>>;

class HttpServerConfiguration
{
private:
    HttpServerConfiguration() = delete;
    std::string serverName_;
    std::string httpVersion_ = "1.1";
    std::map<Url, CallbacksMap> callbacks_;
};

class HttpServerConfigurator
{
public:
    HttpServerConfigurator& withName(std::string serverName);
    HttpServerConfiguration& withHttpVersion(std::string version);
    HttpServerConfigurator& withCallback(HttpMethod method, const Url& url, const std::unique_ptr<ICallback>& callback);

    template<typename ReturnValue, typename... Args>
    HttpServerConfigurator& withCallback(
        HttpMethod method, const Url& url, std::function<ReturnValue(Args...)>&& callback, Args&&... args)
    {
        struct _AnnonymousCallback : public ICallback
        {
            std::function<ReturnValue(Args...)> callback_;
            std::tuple<Args...> args_;
            ReturnValue operator()() { return std::apply(callback_, args_); }
        };
        return withCallback(method, url, std::make_unique<_AnnonymousCallback>(std::move(callback), std::move(args)));
    }

    HttpServerConfiguration build();

private:
    std::string serverName_;
    std::string httpVersion_ = "1.1";
    std::map<Url, CallbacksMap> callbacks_;
};
}  // namespace http
