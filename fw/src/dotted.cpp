#include "dotted.h"

#include <algorithm>
#include <string>

std::string as_dotted(std::string str)
{
    std::replace(str.begin(), str.end(), '/', '.');
    return str;
}
