#include "HttpServer.h"

#include <utility>

namespace http
{
    HttpServer::HttpServer(std::string serverName)
    : socketController_(std::make_unique<SocketController>(std::move(serverName)))
    {
        socketController_->addObserver(this);
    }

    void HttpServer::registerCallback(HttpMethod method, const Url& url, std::unique_ptr<ICallback> callback) 
    {
        callbacks_.emplace(Endpoint{url, method}, std::move(callback));
    }

    void HttpServer::registerCallback(HttpMethod method, const Url& url, std::function<HttpResponse(HttpRequest)> function)
    {
        struct TemporaryCallback : public ICallback
        {
            TemporaryCallback(std::function<HttpResponse(HttpRequest)> function) : function_(std::move(function)){}
            HttpResponse operator()(HttpRequest req) override{ return function_(std::move(req)); }
            std::function<HttpResponse(HttpRequest)> function_;
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
        auto endpoint = Endpoint(Url("/students"), HttpMethod::Get); // get from request
        if(!callbacks_.contains(endpoint))
        {
            //send response with error
            std::cout << "endpoint not set on server" << std::endl;
            return;
        }
        (*callbacks_[endpoint])(HttpRequest{});
    }


}  // namespace http
