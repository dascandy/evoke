#include "Utilities.hpp"

std::string GetNameFromPath(const filesystem::path &path, char separator)
{
    using namespace std;

    auto start = find_if_not(begin(path), end(path), [](auto &part) {
        return part.filename_is_dot();
    });

    if(start == end(path))
    {
        return "#anonymous#";
    }

    string out = start->string();

    while(++start != end(path))
    {
        if(!start->filename_is_dot())
        {
            out.append(1, separator).append(start->string());
        }
    }

    return out;
}