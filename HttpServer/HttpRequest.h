#pragma once

#include <string>
#include <vector>

#include "HtmlHeaders/IHeader.hpp"

namespace http
{
using RequestId = unsigned;
using ClientId = unsigned;

struct HttpRequest
{
    std::string method_;
    std::string url_;
    std::string version_;

    std::vector<std::unique_ptr<IHeader>> headers;
    std::string content;

    RequestId getId() const
    {
        return id_;
    }

    ClientId getSender() const
    {
        return sender_;
    }

    RequestId id_;
    ClientId sender_;
};
}  // namespace http
