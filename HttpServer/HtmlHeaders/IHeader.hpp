#pragma once

#include <concepts>
#include <string>
#include <typeindex>
#include <unordered_map>

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

    virtual IHeader* clone() const = 0;

protected:
    std::string name_;
    std::string value_;
};

template<typename H>
concept HeaderType = std::is_base_of_v<IHeader, H> && requires
{
    {
        H::getName()
        } -> std::same_as<std::string_view>;
};

struct HeadersReader
{
    std::vector<std::unique_ptr<IHeader>> readHeaders()
    {
        std::vector<std::unique_ptr<IHeader>> headers;
        for (auto& [name, value] : headerNameToValue_)
        {
            auto header = headerPrototypes.at(name)->clone();
            header->fromString(value);
            headers.push_back(std::unique_ptr<IHeader>(header));
        }
        return headers;
    }

    template<HeaderType... H>
    void registerHeaders()
    {
        (addHeader<H>(), ...);
    }

    const std::unordered_map<std::string, std::unique_ptr<IHeader>>& get() const
    {
        return headerPrototypes;
    }

private:
    std::vector<std::pair<std::string, std::string>> headerNameToValue_;

    std::unordered_map<std::string, std::unique_ptr<IHeader>> headerPrototypes;

    template<HeaderType H>
    void addHeader()
    {
        headerPrototypes.emplace(H::getName(), std::make_unique<H>());
    }
};

struct H1 : public IHeader
{
    static std::string_view getName()
    {
        return "H1_name";
    }

    std::string toString() const override
    {
        return "";
    }

    void fromString(const std::string& s) override
    {

    }

    IHeader* clone() const
    {
        return new H1(*this);
    }
};

struct H2 : public IHeader
{
    static std::string_view getName()
    {
        return "H2_name";
    }
    std::string toString() const override
    {
        return "";
    }

    void fromString(const std::string& s) override
    {

    }
    IHeader* clone() const
    {
        return new H2(*this);
    }
};

struct H3 : public IHeader
{
    static std::string_view getName()
    {
        return "H3_name";
    }
    std::string toString() const override
    {
        return "";
    }

    void fromString(const std::string& s) override
    {

    }
    IHeader* clone() const
    {
        return new H3(*this);
    }
};
}  // namespace http
