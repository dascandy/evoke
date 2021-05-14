#include "Utilities.hpp"
#include <algorithm>

std::string GetNameFromPath(const fs::path &path, char separator)
{
    using namespace std;

    auto start = std::find_if_not(begin(path), end(path), [](auto &part) {
        return part.filename() == ".";
    });

    if(start == end(path))
    {
        return fs::canonical(path).filename().string();
    }

    string out = start->string();

    while(++start != end(path))
    {
        if(start->filename() != ".")
        {
            out.append(1, separator).append(start->string());
        }
    }

    return out;
}
