#include "FileReader.hpp"

#include <iostream>
#include <filesystem>
#include <fstream>
#include <exception>


std::string FileReader::read(const std::string& fileName)
{
    std::ifstream file(fileName, std::ios_base::ate);
    if(!file.is_open())
    {
        std::cout << "File " + fileName + " does not exist!" << std::endl;
        throw std::ios_base::failure("File " + fileName + " does not exist!");
    }
    auto textSize = file.tellg();
    std::string fileData(textSize, '\0');
    file.seekg(0);
    file.read(fileData.data(), textSize);
    return fileData;
}

