#pragma once

#include <string>

namespace http
{
class Url
{
public:
    Url(std::string address);
    const std::string& getAddress() const;
    bool operator==(const Url&) const = default;

private:
    std::string mainAddress_;
};
}  // namespace http
