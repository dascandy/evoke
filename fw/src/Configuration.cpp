#include "Configuration.h"

#include <fstream>
#include <fw/filesystem.hpp>
#include <iostream>
#include "FileParser.h"

void Configuration::LoadDefaults()
{
    blacklist = {"unity"};
#if defined(_WIN32)
    toolchain = "msvc";
    compileFlags = "/permissive- /std:c++latest";
#elif defined(__APPLE__)
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
    ParseFile("evoke.conf", [](const std::string&){}, [this](const std::string& key, const std::string& value) {
        if(key == "toolchain")
        {
            toolchain = value;
        }
        else if(key == "compile-flags")
        {
            compileFlags = value;
        }
        else if(key == "blocklist")
        {
            blacklist = splitWithQuotes(value);
        }
        else
        {
            std::cout << "Ignoring unknown tag in configuration file: " << key << "\n";
        }

    });
}

Configuration &Configuration::Get()
{
    static Configuration config;
    return config;
}
