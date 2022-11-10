#pragma once

#include "IHeader.hpp"

#define BASIC_HEADER(clsName, headerName)                                                                              \
    class clsName : public IHeader                                                                                     \
    {                                                                                                                  \
    public:                                                                                                            \
        static std::string_view getName()                                                                              \
        {                                                                                                              \
            return headerName;                                                                                         \
        }                                                                                                              \
                                                                                                                       \
        std::string toString() const override                                                                          \
        {                                                                                                              \
            return "";                                                                                                 \
        }                                                                                                              \
                                                                                                                       \
        void fromString(const std::string& s) override                                                                 \
        {                                                                                                              \
            value_ = s;                                                                                                \
        }                                                                                                              \
                                                                                                                       \
        std::unique_ptr<IHeader> clone() const                                                                         \
        {                                                                                                              \
            return std::make_unique<clsName>(*this);                                                                   \
        }                                                                                                              \
    };

namespace http
{
BASIC_HEADER(AIM, "A-IM");
BASIC_HEADER(Accept, "Accept");
BASIC_HEADER(AcceptCharset, "Accept-Charset");
BASIC_HEADER(AcceptDatetime, "Accept-Datetime");
BASIC_HEADER(AcceptEncoding, "Accept-Encoding");
BASIC_HEADER(AcceptLanguage, "Accept-Language");
BASIC_HEADER(AcceptControlRequestMethod, "Accept-Control-Request-Method");
BASIC_HEADER(AcceptControlRequestHeaders, "Accept-Control-Request-Headers");
BASIC_HEADER(Authorization, "Authorization");
BASIC_HEADER(CacheControl, "Cache-Control");
BASIC_HEADER(Connection, "Connection");
BASIC_HEADER(ContentEncoding, "Content-Encoding");
BASIC_HEADER(ContentLength, "Content-Length");
BASIC_HEADER(ContentMD5, "Content-MD5");
BASIC_HEADER(ContentType, "Content-Type");
BASIC_HEADER(Date, "Date");
BASIC_HEADER(Expect, "Expect");

BASIC_HEADER(Forwarded, "Forwarded");
BASIC_HEADER(From, "From");
BASIC_HEADER(Host, "Host");
BASIC_HEADER(HTTP2Settings, "HTTP2-Settings");
BASIC_HEADER(IfMatch, "If-Match");
BASIC_HEADER(IfModifiedSince, "If-Modified-Since");
BASIC_HEADER(IfNoneMatch, "If-None-Match");
BASIC_HEADER(IfRange, "If-Range");
BASIC_HEADER(IfUnmodifiedSince, "If-Unmodified-Since");
BASIC_HEADER(MaxForwards, "Max-Forwards");
BASIC_HEADER(Origin, "Origin");
BASIC_HEADER(Pragma, "Pragma");
BASIC_HEADER(Prefer, "Prefer");
BASIC_HEADER(ProxyAuthorization, "Proxy-Authorization");
BASIC_HEADER(Range, "Range");

BASIC_HEADER(Referer, "Referer");
BASIC_HEADER(TE, "TE");
BASIC_HEADER(Trailer, "Trailer");
BASIC_HEADER(TransferEncoding, "Transfer-Encoding");
BASIC_HEADER(UserAgent, "User-Agent");
BASIC_HEADER(Upgrade, "Upgrade");
BASIC_HEADER(Via, "Via");
BASIC_HEADER(Warning, "Warning");
}  // namespace http
