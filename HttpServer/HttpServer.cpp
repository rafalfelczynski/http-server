#include "HttpServer.h"

#include <utility>

#include "eventsystem/EventSystem.hpp"
#include <ranges>
#include <string_view>
#include "HtmlHeaders/HeaderReader.h"

namespace http
{
HttpServer::HttpServer(std::string serverName)
    : socketController_(std::make_unique<SocketController>(std::move(serverName), "http"))
{
    socketController_->setHttpListener(this);
    // EventSystem::getInstance().listen(this, ClientDataReceivedEvent::EVENT_TYPE);
}

HttpServer::~HttpServer()
{
    std::cout << "server removed" << std::endl;
}

void HttpServer::registerCallback(HttpMethod method, const Url& url, std::unique_ptr<ICallback> callback)
{
    callbacks_.emplace(Endpoint{url, method}, std::move(callback));
}

void HttpServer::registerCallback(HttpMethod method, const Url& url, CallbackFcn function)
{
    struct TemporaryCallback : public ICallback
    {
        TemporaryCallback(CallbackFcn function)
            : function_(std::move(function))
        {
        }
        std::string operator()(const HttpRequest& req) override
        {
            return function_(std::move(req));
        }
        CallbackFcn function_;
    };
    std::unique_ptr<ICallback> callback = std::make_unique<TemporaryCallback>(std::move(function));
    registerCallback(method, url, std::move(callback));
}

void HttpServer::run()
{
    socketController_->start();
    // do sth
    // wait on condition variable instead of joining
    socketController_->join();
    /* while(isRunning)
    {
        cv.wait();
    }
    */
}

HttpResponse HttpServer::onHttpRequest(const HttpRequest& request)
{
    // parse string to http request
    // validate request and call callback
    auto lines = request.content | std::ranges::views::lazy_split('\n');
    std::cout << "lines empty: " << lines.empty();
    const auto& line = lines.front();
    auto words = line | std::ranges::views::lazy_split(' ') | std::ranges::views::transform([](auto &&rng) {
            return std::string_view(&*rng.begin(), std::ranges::distance(rng));
    });
    const auto& method = *words.begin();
    const auto& url = *std::next(words.begin());
    std::cout << "url: " << url << std::endl;


    auto endpoint = Endpoint(Url(std::string(url)), HttpMethod::Get);  // get from request
    if (!callbacks_.count(endpoint))
    {
        // send response with error
        std::cout << "endpoint not set on server" << std::endl;
        std::string notFoundPage = "<html>"
"<head><title>404 Not Found</title></head>"
"<body bgcolor=\"white\">"
"<center><h1>404 Not Found</h1></center>"
"<hr><center>nginx/0.8.54</center>"
"</body>"
"</html>";
        // socketController_->sendResponse(request.getId(), HttpResponse{"HTTP/1.1 404 Not Found\r\n"
        //             "Cache-Control: no-cache, private\r\n"
        //             "Content-Type: text/html\r\n"
        //             "Content-Length: " + std::to_string(notFoundPage.size()) + "\r\n"
        //             "\r\n"
        //             + notFoundPage});
        return HttpResponse{"HTTP/1.1 404 Not Found\r\n"
                    "Cache-Control: no-cache, private\r\n"
                    "Content-Type: text/html\r\n"
                    "Content-Length: " + std::to_string(notFoundPage.size()) + "\r\n"
                    "\r\n"
                    + notFoundPage};
    }
    auto responseStr = (*callbacks_[endpoint])(request);
    return HttpResponse{responseStr};
    //socketController_->sendResponse(request.getId(), HttpResponse{responseStr});
}
}  // namespace http
