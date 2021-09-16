#pragma once

#include <string>

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
    virtual ResultCode operator()() = 0;
    inline const std::string& getLastError(){ return errorDescription_; };

protected:
    std::string errorDescription_;
};
}  // namespace http
