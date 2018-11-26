#include <string>
#include "dotted.h"
#include <algorithm>

std::string as_dotted(std::string str)
{
    std::replace(str.begin(), str.end(), '/', '.');
    return str;
}

