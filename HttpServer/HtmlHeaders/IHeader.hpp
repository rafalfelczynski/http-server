#pragma once

#include <string>

namespace http
{
struct IHeader
{
    virtual ~IHeader() = default;
    virtual std::string prepare() = 0;
};
}  // namespace http
