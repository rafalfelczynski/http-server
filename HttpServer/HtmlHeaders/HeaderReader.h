#pragma once

#include <unordered_map>

#include "IHeader.hpp"
#include "RequestHeaders.h"

namespace http
{
template<typename H>
concept HeaderType = std::is_base_of_v<IHeader, H> && requires
{
    {
        H::getName()
        } -> std::same_as<std::string_view>;
};

struct HeadersReader
{
    HeadersReader()
    {
        registerHeaders<
            AIM,
            Accept,
            AcceptCharset,
            AcceptDatetime,
            AcceptEncoding,
            AcceptLanguage,
            AcceptControlRequestMethod,
            AcceptControlRequestHeaders,
            Authorization,
            CacheControl,
            Connection,
            ContentEncoding,
            ContentLength,
            ContentMD5,
            ContentType,
            Date,
            Expect,

            Forwarded,
            From,
            Host,
            HTTP2Settings,
            IfMatch,
            IfModifiedSince,
            IfNoneMatch,
            IfRange,
            IfUnmodifiedSince,
            MaxForwards,
            Origin,
            Pragma,
            Prefer,
            ProxyAuthorization,
            Range,

            Referer,
            TE,
            Trailer,
            TransferEncoding,
            UserAgent,
            Upgrade,
            Via,
            Warning
            >();
    }

    std::vector<std::unique_ptr<IHeader>> readHeaders(const std::string_view& httpMessage)
    {
        std::vector<std::unique_ptr<IHeader>> headers;
        for (auto& [name, value] : headerNameToValue_)
        {
            auto header = headerPrototypes.at(name)->clone();
            header->fromString(value);
            headers.push_back(std::move(header));
        }
        return headers;
    }

    std::unique_ptr<IHeader> readHeader(const std::string_view& name, const std::string_view& value)
    {
        std::cout << "header name: " << name << " size: " << name.size() << " has: " << headerPrototypes.count(std::string(name)) << std::endl;
        if(headerPrototypes.count(std::string(name)))
        {
            auto header = headerPrototypes.at(std::string(name))->clone();
            header->fromString(std::string(value));
            return header;
        }
        return {};
    }

    const std::unordered_map<std::string, std::unique_ptr<IHeader>>& get() const
    {
        return headerPrototypes;
    }

private:
    std::vector<std::pair<std::string, std::string>> headerNameToValue_;

    std::unordered_map<std::string, std::unique_ptr<IHeader>> headerPrototypes;

    template<HeaderType... H>
    void registerHeaders()
    {
        (addHeader<H>(), ...);
    }

    template<HeaderType H>
    void addHeader()
    {
        headerPrototypes.emplace(H::getName(), std::make_unique<H>());
    }
};
}  // namespace http