#pragma once

#include <thread>

#include "HttpRequest.h"

namespace http
{
class RequestWorker
{
public:
    RequestWorker(HttpRequest&& request);

private:
    HttpRequest request_;
    std::thread worekrThread_;
};
}  // namespace http
