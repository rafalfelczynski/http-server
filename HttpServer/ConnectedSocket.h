#pragma once

#include <string>

#include <WinSock2.h>
#include "ThreadPool.h"

#include "MovableMutex.h"

namespace http
{

struct ISocketDataListener
{
    virtual ~ISocketDataListener() = default;
    virtual std::string onSocketDataReceived(unsigned socketId, const std::string& data) = 0;
    virtual void onConnectionClosed(unsigned clientId) = 0;
};


class ConnectedSocket
{
public:
    ConnectedSocket(unsigned clientId, SOCKET socket);
    ~ConnectedSocket();
    void start();
    bool operator==(const ConnectedSocket& oth) const;
    SOCKET getSocket() const;

    unsigned getId() const
    {
        return clientId_;
    }

    bool isFinished() const
    {
        return readingFinished_;
    }

    bool isClosed() const;

    void close();

    void setListener(ISocketDataListener* listener)
    {
        this->listener_ = listener;
    }

    void removeListener()
    {
        this->listener_ = nullptr;
    }

private:
    std::string receiveData();
    void sendData(const std::string& msg);
    void run();
    int receivePartOfMsg(std::string& receivedMsg);
    void waitForDataToArrive();
    bool isReadyToRead() const;
    void updateSocketStateFlags(int numOfReadBytes);

    unsigned clientId_;
    SOCKET client_;
    std::atomic_bool readingFinished_;
    std::atomic_bool closed_;
    std::unique_ptr<ThreadPool> workers_;
    ISocketDataListener* listener_ = nullptr;
};
}  // namespace http
