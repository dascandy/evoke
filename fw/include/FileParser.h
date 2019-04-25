#pragma once

#include <string>
#include <vector>
#include <functional>

std::vector<std::string> splitWithQuotes(const std::string &str);
void ParseFile(const std::string& fileName, std::function<void(const std::string&)> onTag, std::function<void(const std::string&, const std::string&)> onKeyValue);



