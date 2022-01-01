#include <fstream>
#include <fw/filesystem.hpp>
#include <iostream>
#include <functional>
#include <string>
#include <vector>

std::vector<std::string> splitWithQuotes(const std::string &str)
{
    std::vector<std::string> rv;
    const char *s = &str[0], *e = (&str.back()) + 1;
    const char *p = s;
    bool inQuotes = false;
    while(s < e)
    {
        if(*s == ' ' && !inQuotes)
        {
            if(*p != '"')
                rv.push_back(std::string(p, s));
            else
                rv.push_back(std::string(p + 1, s - 1));

            p = s + 1;
        }
        else if(*s == '\"')
        {
            inQuotes = !inQuotes;
        }
        s++;
    }
    if(p != s) {
        if(*p != '"')
            rv.push_back(std::string(p, s));
        else
            rv.push_back(std::string(p + 1, s - 1));
    }

    return rv;
}

void ParseFile(const std::string& fileName, std::function<void(const std::string&)> onTag, std::function<void(const std::string&, const std::string&)> onKeyValue) {
    std::ifstream in(fileName);
    std::string line;
    while(in.good())
    {
        std::getline(in, line);
        while(in.good() && !line.empty() && line.back() == '\\')
        {
            std::string nextLine;
            std::getline(in, nextLine);
            line += nextLine;
        }

        size_t pos = line.find_first_of("#");
        if(pos != std::string::npos)
            line.resize(line.find_first_of("#"));

        size_t firstChar = line.find_first_not_of(" \t\n"), lastChar = line.find_last_not_of(" \t\n");
        if (firstChar == std::string::npos) continue;
        line = line.substr(firstChar, lastChar - firstChar + 1);

        if (pos = line.find(": "); pos != std::string::npos) {
            std::string name = line.substr(0, pos);
            std::string value = line.substr(pos + 2);
            onKeyValue(name, value);
        } else if (!line.empty() && line.front() == '[' && line.back() == ']') {
            onTag(line.substr(1, line.size() - 2));
        } else {
            // TODO: yell
        }
    }
}

