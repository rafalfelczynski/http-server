#include "HtmlDecoder.hpp"

#include <iostream>
#include <numeric>
#include <regex>
#include <variant>
#include <vector>

#include <HttpServer/Helpers.hpp>

std::vector<Placeholder> HtmlDecoder::findFragmentsWithDataPlaceholders(const std::string& text)
{
    static const auto seq = std::string("@(\\w+).(\\w+)");
    static const std::regex word_regex(seq);
    static std::smatch matches;
    size_t lastMatchEndPos = 0;

    std::vector<Placeholder> fragments;
    while (std::regex_search(text.begin() + lastMatchEndPos, text.end(), matches, word_regex))
    {
        fragments.emplace_back(matches.prefix().str());
        fragments.emplace_back(DataPlaceholder{matches[1].str(), matches[2].str()});
        lastMatchEndPos += matches.position(0) + matches.length(0);
    }
    fragments.emplace_back(std::string(text.begin() + lastMatchEndPos, text.end()));
    return fragments;
}

std::string HtmlDecoder::joinFragments(std::vector<Placeholder>&& fragments, const std::unordered_map<std::string, std::string>& data)
{
    std::string textCopy;

    auto copyStr = [&textCopy](std::string& str) { textCopy += std::move(str); };
    auto fillPlaceholder = [&textCopy, &data](const DataPlaceholder& place) {
        auto valueIt = data.find(place.keyName);
        if (valueIt != data.end())
        {
            textCopy.append(valueIt->second);
        }
        else
        {
            textCopy.append("@" + place.mapName + "." + place.keyName);
        }
    };

    auto visitor = helpers::compose{std::move(copyStr), std::move(fillPlaceholder)};
    for (auto& pl : fragments)
    {
        std::visit(visitor, pl);
    }
    return textCopy;
}

std::string HtmlDecoder::replace(const std::string& text, const std::unordered_map<std::string, std::string>& data)
{
    auto fragments = findFragmentsWithDataPlaceholders(text);
    return joinFragments(std::move(fragments), data);
}
