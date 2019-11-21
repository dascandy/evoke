#include "fw/dotted.h"

#include <algorithm>
#include <string>

std::string as_dotted(std::string str)
{
    std::replace(str.begin(), str.end(), '/', '.');
    return str;
}

fs::path removeDot(fs::path const &p)
{
    if (p.begin()->filename() == ".")
    {
        return relative(p, ".");
    }
    return p;
}
