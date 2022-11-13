#pragma once

#include <unordered_map>

#include "IHeader.hpp"
#include "RequestHeaders.h"
#include "Utils.h"

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
            Warning>();
    }

    std::vector<std::unique_ptr<IHeader>> readHeaders(const std::string_view& httpMessage)
    {
        std::vector<std::unique_ptr<IHeader>> headers;

        constexpr std::string_view HEADER_LINE_END{"\r\n"};
        auto lines = httpMessage | std::ranges::views::lazy_split(HEADER_LINE_END);
        for (const auto& line :
             lines | std::views::drop(1) | std::views::take_while([](const auto& line) { return !line.empty(); }))
        {
            auto lineView = std::string_view(&*line.begin(), std::ranges::distance(line));
            auto header = readHeader(lineView);
            if (header)
            {
                headers.push_back(std::move(header));
            }
        }
        return headers;
    }

    std::unique_ptr<IHeader> readHeader(std::string_view lineView)
    {
        auto splitLine = lineView | std::ranges::views::split(':') | std::views::transform([](const auto& el) {
                             auto view = std::string_view(&*el.begin(), el.size());
                             return utils::trim(view);
                         });
        if (std::ranges::distance(splitLine) == 2)
        {
            auto fragmentsIt = splitLine.begin();
            auto headerName = std::string(*fragmentsIt++);
            auto value = std::string(*fragmentsIt);
            auto prototypeIt = headerPrototypes.find(headerName);
            if (prototypeIt != headerPrototypes.end())
            {
                auto header = prototypeIt->second->clone();
                header->fromString(value);
                return header;
            }
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