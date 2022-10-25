#pragma once

#include <string>

#include <WinSock2.h>
#include "ThreadPool.h"

namespace http
{
class ConnectedSocket
{
public:
    ConnectedSocket(unsigned clientId, SOCKET socket);
    std::string receiveData() const;
    void sendData(const std::string& msg) const;
    bool operator==(const ConnectedSocket& oth) const;
    SOCKET getSocket() const;

    unsigned getId() const
    {
        return clientId_;
    }

    bool isFinished() const
    {
        return finished_;
    }

    void close();

private:
    void run();
    int receivePartOfMsg(std::string& receivedMsg) const;
    void waitForDataToArrive() const;
    bool hasMoreDataToRead(int lastNumOfBytes) const;

    unsigned clientId_;
    SOCKET client_;
    mutable bool finished_;

    WorkerThread worker_;
};
}  // namespace http
