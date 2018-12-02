#pragma once

#include <string>
#include <vector>

struct GlobalOptions
{
    std::vector<std::string> compile;
    std::vector<std::string> include;
    std::vector<std::string> link;
};
