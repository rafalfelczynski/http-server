#include "SocketController.h"

#include <algorithm>
#include <ranges>

#include "ThirdParty/Timer.hpp"

#include "HtmlHeaders/HeaderReader.h"
#include "Utils.h"

namespace http
{
ConnectionHandler::ConnectionHandler(
    const std::string& hostNameOrAddress,
    const std::string& serviceNameOrPort,
    const std::shared_ptr<SocketClientsHolder>& clientsHolder)
    : clientsHolder_(clientsHolder)
    , listeningSocket_(std::make_shared<ListeningSocket>(hostNameOrAddress, serviceNameOrPort))
    , isListening_(false)
{
    listeningSocket_->setClientsHolder(clientsHolder_);
}

ConnectionHandler::~ConnectionHandler()
{
    removeListener();
    isListening_ = false;
    listeningSocket_->release();
    if (worker_)
    {
        std::cout << "worker joining" << std::endl;
        worker_->joinAll();
    }
    std::cout << "all workers joined" << std::endl;
}

void ConnectionHandler::start()
{
    if (worker_)
    {
        return;
    }
    listeningSocket_->bind();
    if (!listeningSocket_->isBound())
    {
        return;
    }
    std::cout << "listener started" << std::endl;
    worker_ = std::make_unique<ThreadPool>(1);
    worker_->init();
    isListening_ = true;
    worker_->process([this]() {
        while (isListening_)
        {
            std::cout << "waiting for client to connect" << std::endl;
            auto clientId = listeningSocket_->waitForClientToConnect();
            if (clientId)
            {
                std::cout << "client connected with id: " << *clientId << std::endl;
                if (listener_)
                {
                    listener_->onClientConnected(*clientId);
                }
            }
        }
    });
}

void ConnectionHandler::stop()
{
    std::cout << "listener stopped" << std::endl;
    isListening_ = false;
    if (worker_)
    {
        worker_->joinAll();
    }
}

void ConnectionHandler::join()
{
    std::cout << "listener joined" << std::endl;
    if (worker_)
    {
        worker_->joinAll();
    }
}

void ConnectionHandler::setListener(IListener* listener)
{
    this->listener_ = listener;
}

void ConnectionHandler::removeListener()
{
    this->listener_ = nullptr;
}

SocketController::SocketController(const std::string& hostNameOrAddress, const std::string& serviceNameOrPort)
    : clientsHolder_(std::make_shared<SocketClientsHolder>())
    , connectionHandler_(hostNameOrAddress, serviceNameOrPort, clientsHolder_)
{
    connectionHandler_.setListener(this);
}

SocketController::~SocketController()
{
    httpReqListener_ = nullptr;
    std::cout << "socket controller destr" << std::endl;
}

void SocketController::start()
{
    if (!httpReqListener_)
    {
        std::cout << "[CRITICAL] HttpRequestListener not initialized!" << std::endl;
        return;
    }
    connectionHandler_.start();
}

void SocketController::join()
{
    connectionHandler_.join();
}

// void SocketController::sendResponse(RequestId requestId, const HttpResponse& msg)
// {
//     // parse http response to string to send via socket
//     auto clientId = requestToClientMap_[requestId];
//     auto client = clientsHolder_->getClient(clientId);
//     client->sendData(msg.msg);
//     requestToClientMap_.erase(requestId);
// }

void SocketController::setHttpListener(IHttpRequestListener* listener)
{
    httpReqListener_ = listener;
}

void SocketController::onClientConnected(unsigned clientId)
{
    auto client = clientsHolder_->getClient(clientId);
    client->setListener(this);
    client->start();
}

void SocketController::onConnectionClosed(unsigned clientId)
{
    // workaround
    timer::SingleShotTimer::call(100, [this, clientId]() { clientsHolder_->removeClient(clientId); });
}

std::string SocketController::onSocketDataReceived(unsigned clientId, const std::string& data)
{
    auto request = clientDataParser_.parseData(clientId, data);
    requestToClientMap_.emplace(request.getId(), request.getSender());
    if (httpReqListener_)
    {
        auto response = httpReqListener_->onHttpRequest(request);
        return response.msg;  // response.toString();
        // auto client = clientsHolder_->getClient(clientId);
        // client->sendData(response.msg);
        // requestToClientMap_.erase(request.getId()); // may not be needed
    }
    return "";
}

// TODO: should not be there, move

HttpRequest ClientDataParser::parseData(unsigned clientId, const std::string& clientData)  // maybe move, not const ref
{
    static RequestId requestId = 0;

    // builder needed
    constexpr std::string_view HEADERS_AND_DATA_DELIM{"\r\n\r\n"};
    auto contentStart = clientData.find(HEADERS_AND_DATA_DELIM) + HEADERS_AND_DATA_DELIM.size();

    auto req = HttpRequest();
    readUri(req, clientData);
    HeadersReader reader;
    req.headers = reader.readHeaders(std::string_view(clientData.data(), contentStart));

    std::string_view content{};
    if(contentStart < clientData.size())
    {
        content = std::string_view(clientData.begin() +contentStart, clientData.end());
    }

    std::cout << "content start: " << contentStart << " content size: " << content.size() << std::endl;


    req.id_ = requestId++;
    req.sender_ = clientId;
    req.content = clientData;
    return req;
}

void ClientDataParser::readUri(HttpRequest& req, const std::string& clientData)
{
    constexpr std::string_view HEADER_LINE_END{"\r\n"};
    auto pos = clientData.find(HEADER_LINE_END);
    if(pos == std::string::npos)
    {
        return;
    }
    auto firstLine = std::string_view(clientData.data(), pos);

    auto words = firstLine | std::ranges::views::split(' ') | std::ranges::views::transform([](const auto &rng) {
        return std::string_view(rng.begin(), rng.end());
    }) | std::views::take(3);
    if(std::ranges::distance(words) != 3)
    {
        return;
    }

    auto wordsIt = words.begin();
    req.method_ = *wordsIt++;
    req.url_ = *wordsIt++;
    req.version_ = utils::trim(*wordsIt);


}
}  // namespace http
