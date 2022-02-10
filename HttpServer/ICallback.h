#pragma once

#include <string>
#include "HttpRequest.h"
#include "HttpResponse.h"

namespace http
{
class ICallback
{
public:
    enum class ResultCode
    {
        CallbackOk,
        ErrorDuringCallback
    };
    virtual ~ICallback() = default;
    virtual std::string operator()(HttpRequest req) = 0;
    inline const std::string& getLastError(){ return errorDescription_; };

protected:
    std::string errorDescription_;
};
}  // namespace http
