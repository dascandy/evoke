#include <string>
#include "dotted.h"
#include <algorithm>

std::string as_dotted(const std::string& str)
{
    std::string rv = str;
    std::replace(rv.begin(), rv.end(), '/', '.');
    return rv;
}

