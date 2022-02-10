#pragma once

#include <string>
#include <vector>
#include <variant>
#include <unordered_map>

struct DataPlaceholder
{
    std::string mapName;
    std::string keyName;
};

using Placeholder = std::variant<std::string, DataPlaceholder>;

class HtmlDecoder
{
public:
    std::string replace(const std::string& message, const std::unordered_map<std::string, std::string>& data);
private:
    std::vector<Placeholder> findFragmentsWithDataPlaceholders(const std::string& text);
    std::string joinFragments(std::vector<Placeholder>&& fragments, const std::unordered_map<std::string, std::string>& data);
};
