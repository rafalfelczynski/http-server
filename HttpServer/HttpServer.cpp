#include "HttpServer.h"

#include <utility>

namespace http
{
    using CallbackFcn = std::function<std::string(HttpRequest)>;

    HttpServer::HttpServer(std::string serverName)
    : socketController_(std::make_unique<SocketController>(std::move(serverName), "http"))
    {
        socketController_->addObserver(this);
    }

    void HttpServer::registerCallback(HttpMethod method, const Url& url, std::unique_ptr<ICallback> callback) 
    {
        callbacks_.emplace(Endpoint{url, method}, std::move(callback));
    }

    void HttpServer::registerCallback(HttpMethod method, const Url& url, CallbackFcn function)
    {
        struct TemporaryCallback : public ICallback
        {
            TemporaryCallback(CallbackFcn function) : function_(std::move(function)){}
            std::string operator()(HttpRequest req) override{ return function_(std::move(req)); }
            CallbackFcn function_;
        };
        std::unique_ptr<ICallback> callback = std::make_unique<TemporaryCallback>(std::move(function));
        registerCallback(method, url, std::move(callback));
    }

    void HttpServer::run()
    {
        // do sth
        // wait on condition variable instead of joining
        socketController_->join();
    }
    
    void HttpServer::onPublisherNotification(const ReceivedClientData& clientData) // called when successfully received dava from socket client
    {
        // parse string to http request
        // validate request and call callback
        std::cout << "data received" << std::endl;
        auto endpoint = Endpoint(Url("/students"), HttpMethod::Get); // get from request
        if(!callbacks_.contains(endpoint))
        {
            //send response with error
            std::cout << "endpoint not set on server" << std::endl;
            return;
        }
        auto responseStr = (*callbacks_[endpoint])(HttpRequest{});
        socketController_->sendBack(clientData.clientId, responseStr);
    }


}  // namespace http
