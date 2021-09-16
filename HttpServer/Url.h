#pragma once

#include <string>

namespace http
{
class Url
{
public:
    Url(std::string address);
    const std::string& getAddress() const;

private:
    std::string address_;
};
}  // namespace http
