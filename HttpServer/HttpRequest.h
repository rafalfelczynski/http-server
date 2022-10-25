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
    friend class HttpRequestBuilder;

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

class HttpRequestBuilder
{
public:
    HttpRequest build()
    {
        // auto content = HtmlHeadersParser().prepare(headers_);
        std::string content;
        return HttpRequest{std::move(headers_), content};
    }

    HttpRequestBuilder& withHeader(std::unique_ptr<IHeader> header)
    {
        headers_.emplace_back(std::move(header));
        return *this;
    }

private:
    std::vector<std::unique_ptr<IHeader>> headers_;
};
}  // namespace http
