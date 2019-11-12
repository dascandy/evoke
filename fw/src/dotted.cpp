#include "dotted.h"

#include <algorithm>
#include <string>

std::string as_dotted(std::string str)
{
    std::replace(str.begin(), str.end(), '/', '.');
    return str;
}

filesystem::path removeDot(filesystem::path const &p)
{
    if (p.begin()->filename() == ".")
    {
        return relative(p, ".");
    }
    return p;
}
