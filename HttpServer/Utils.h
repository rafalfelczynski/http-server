#pragma once

#include <string_view>
#include <cctype>

namespace utils
{
inline std::string_view trim(std::string_view view)
{
    while (view.size() > 0 && std::isspace(view.front()))
    {
        view.remove_prefix(1);
    }

    while (view.size() > 0 && std::isspace(view.back()))
    {
        view.remove_suffix(1);
    }
    return view;
}
}  // namespace utils
