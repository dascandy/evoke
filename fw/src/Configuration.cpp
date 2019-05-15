#include "Configuration.h"

#include <fstream>
#include <fw/filesystem.hpp>
#include <iostream>

static std::vector<std::string> splitWithQuotes(const std::string &str)
{
    std::vector<std::string> rv;
    const char *s = &str[0], *e = &str[str.size() - 1];
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
    if(p != s)
        rv.push_back(std::string(p, s));
    return rv;
}

void Configuration::LoadDefaults()
{
    blacklist = {"unity"};
#if defined(_WIN32)
    toolchain = "msvc";
    compileFlags = "/permissive- /std:c++latest";
#elif defined(APPLE)
    toolchain = "clang";
    compileFlags = "-std=c++17";
#else
    toolchain = "gcc";
    compileFlags = "-std=c++17 -pthread";
#endif
}

Configuration::Configuration()
{
    LoadDefaults();
    std::ifstream in("evoke.conf");
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

        pos = line.find(": ");
        if(pos == std::string::npos)
            continue;

        std::string name = line.substr(0, pos);
        std::string value = line.substr(pos + 2);
        if(name == "toolchain")
        {
            toolchain = value;
        }
        else if(name == "compile-flags")
        {
            compileFlags = value;
        }
        else if(name == "blocklist")
        {
            blacklist = splitWithQuotes(value);
        }
        else
        {
            std::cout << "Ignoring unknown tag in configuration file: " << name << "\n";
        }
    }
}

Configuration &Configuration::Get()
{
    static Configuration config;
    return config;
}
