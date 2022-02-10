#pragma once

#include <string>
#include <optional>

class FileReader
{
public:
    static std::string read(const std::string& fileName); 
};
