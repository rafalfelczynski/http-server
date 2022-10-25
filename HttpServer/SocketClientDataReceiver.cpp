#include "SocketClientDataReceiver.h"

#include "eventsystem/ClientDataReceivedEvent.hpp"
#include "eventsystem/EventSystem.hpp"

namespace http
{
namespace
{
constexpr int DEFAULT_NUMBER_OF_THREADS = 5;
}
SocketClientDataController::SocketClientDataController(
    const std::shared_ptr<ListeningSocket>& socket, const std::shared_ptr<SocketClientsHolder>& clientsHolder)
    : socket_(socket)
    , dataReceiverWorkers_(DEFAULT_NUMBER_OF_THREADS)
    , httpDataParserWorkers_(2)
    , clientsHolder_(clientsHolder)
{
}

SocketClientDataController::~SocketClientDataController()
{
    removeListener();
}

void SocketClientDataController::asyncReceiveData(unsigned clientId)
{
    std::cout << "process async receive data client: " << clientId << std::endl;
    dataReceiverWorkers_.process([this, clientId]() {
        std::cout << "receiving data..... client " << clientId << std::endl;
        auto client = clientsHolder_->getClient(clientId);
        do
        {
            auto msg = client->receiveData();
            std::cout << "client: " << clientId << " receiving data finished!! Message size: " << msg.size()
                      << std::endl;
            if (!msg.empty())
            {
                httpDataParserWorkers_.process([this, clientId, msg = std::move(msg)]() {
                    auto request = clientDataParser_.parseData(ReceivedClientData{clientId, std::move(msg)});
                    if (listener_)
                    {
                        listener_->onRequestReceived(request);
                    }
                });
                // EventSystem::getInstance().post<ClientDataReceivedEvent>(ReceivedClientData{clientId,
                // std::move(*msg)}, this);
            }
            else
            {
                if (client->isFinished())
                {
                    client->close();  // if connection is not "keep-alive"
                }
            }
        } while (!client->isFinished());
    });
    // do sth with data
}

void SocketClientDataController::asyncSendData(unsigned clientId, const std::string& data)
{
    std::cout << "process async send data to client: " << clientId << std::endl;
    // async receive
    dataReceiverWorkers_.process([this, clientId, data]() {
        std::cout << "Sending back data of length: " << data.size() << " to client: " << clientId << std::endl;
        auto client = clientsHolder_->getClient(clientId);
        client->sendData(data);  // request

        if (client->isFinished())
        {
            client->close(); // if connection is not "keep-alive"
        }
    });
    // do sth with data
}

void SocketClientDataController::setListener(IListener* listener)
{
    listener_ = listener;
}

void SocketClientDataController::removeListener()
{
    listener_ = nullptr;
}

}  // namespace http