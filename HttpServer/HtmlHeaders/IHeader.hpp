#pragma once

#include <concepts>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <memory>

namespace http
{
struct IHeaderStreamWriter
{
    virtual ~IHeaderStreamWriter() = 0;
    virtual void writeValue(const std::string& value) = 0;
};

struct IHeaderStreamReader
{
    virtual ~IHeaderStreamReader() = 0;
    virtual std::string readValue(const std::string& name) = 0;
};

struct IHeader
{
    virtual ~IHeader() = default;
    virtual std::string toString() const = 0;
    virtual void fromString(const std::string& str) = 0;

    virtual std::unique_ptr<IHeader> clone() const = 0;

    const std::string& getValue() const
    {
        return value_;
    }

protected:
    std::string value_;
};
}  // namespace http
