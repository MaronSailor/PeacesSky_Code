#pragma once
#include <iostream>
#include <fstream>
#include <sstream>

static std::string readTextFile(const std::string& filePath)
{
    std::ifstream readData(filePath);
    if (!readData)
    {
        std::cout << "error while reading text file\n";
        return {};
    }
    std::stringstream stringStr;
    stringStr << readData.rdbuf();
    return stringStr.str();
}