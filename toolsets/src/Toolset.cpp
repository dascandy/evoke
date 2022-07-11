#include "Toolset.h"

#include "Component.h"
#include "fw/dotted.h"
#include "fw/sha512.h"

#include <fstream>
#include <fw/filesystem.hpp>
#include <string>
#include <cstdlib>

std::string Toolset::getNameFor(const Component &component)
{
    return component.GetName();
}

std::unique_ptr<Toolset> ParseToolset(const std::string &name);

// TODO: actually hash the compiler executable
std::unique_ptr<Toolset> GetToolsetByName(const std::string &name)
{
    static std::string homefolder = getenv("HOME");
    std::unique_ptr<Toolset> toolset;
    if(fs::is_regular_file(std::getenv("HOME") + std::string("/.evoke/") + name + ".toolset") && name.substr(0, 10) != "__builtin_")
    {
        toolset = ParseToolset(std::getenv("HOME") + std::string("/.evoke/") + name + ".toolset");
    }
    else if(fs::is_regular_file("toolsets/" + name + ".toolset") && name.substr(0, 10) != "__builtin_")
    {
        toolset = ParseToolset("toolsets/" + name + ".toolset");
    }
    else if(fs::is_regular_file(homefolder + "/.evoke/" + name + ".toolset") && name.substr(0, 10) != "__builtin_")
    {
        toolset = ParseToolset(homefolder + "/.evoke/" + name + ".toolset");
    }
    else if(name == "windows" || name == "msvc" || name == "__builtin_msvc")
    {
        toolset = std::make_unique<MsvcToolset>();
        toolset->hash = { 0x94, 0xf8, 0xef, 0xfd, 0x72, 0x45, 0x4e, 0x0c, 0x17, 0xe5, 0x59, 0xc9, 0x5d, 0x82, 0xf5, 0xe8, 0x24, 0x60, 0x8a, 0x4d, 0xd9, 0xd3, 0x32, 0xce, 0xff, 0x67, 0x5f, 0x87, 0x4c, 0xce, 0x82, 0x82, 0xd0, 0x69, 0x81, 0x0f, 0x09, 0x07, 0xf2, 0xa7, 0x61, 0x30, 0xf0, 0x49, 0x81, 0xcc, 0x54, 0x33, 0xcd, 0x14, 0xfd, 0xb7, 0x92, 0x67, 0xb7, 0x9a, 0x29, 0xb8, 0x78, 0x28, 0x2e, 0x8c, 0xe2, 0x34 };
    }
    else if(name == "apple" || name == "osx" || name == "clang" || name == "__builtin_clang")
    {
        toolset = std::make_unique<ClangToolset>();
        toolset->hash = { 0xd7, 0x06, 0x0f, 0x38, 0x2d, 0x7d, 0x29, 0x0c, 0x68, 0xdd, 0xcc, 0x0d, 0x3e, 0x3f, 0x43, 0x31, 0xd8, 0x54, 0x7c, 0x2b, 0xe4, 0x00, 0xde, 0x8f, 0xa1, 0xbd, 0x4b, 0x51, 0xa5, 0x7d, 0xd6, 0xd5, 0x71, 0x1d, 0xd1, 0x1f, 0xee, 0x13, 0xd4, 0x5d, 0xc5, 0xcd, 0xbe, 0x21, 0xc1, 0x1a, 0xd8, 0x01, 0x3b, 0xd6, 0x2e, 0x29, 0x8e, 0x72, 0x52, 0x32, 0x2e, 0x6f, 0x3d, 0xd9, 0xac, 0x5c, 0x97, 0x53 };
    }
    else if(name == "linux" || name == "gcc" || name == "__builtin_gcc")
    {
        toolset = std::make_unique<GccToolset>();
        toolset->hash = { 0xc9, 0x08, 0xd6, 0x4b, 0x5d, 0xe8, 0x92, 0xa8, 0xdd, 0x7c, 0x58, 0x19, 0xd9, 0x4a, 0x52, 0x9c, 0x41, 0xe1, 0x49, 0x44, 0x17, 0xff, 0xcf, 0x21, 0x1a, 0x84, 0x08, 0x37, 0x08, 0x6a, 0xb4, 0x4b, 0x81, 0xaa, 0x57, 0xca, 0x7f, 0x93, 0x74, 0x56, 0x86, 0x02, 0x54, 0xd0, 0xa6, 0xb1, 0x96, 0xca, 0x63, 0x20, 0x9a, 0xee, 0xfc, 0xbf, 0xeb, 0x4d, 0x87, 0x0e, 0xcb, 0x08, 0x5a, 0xf5, 0xd9, 0x6d };
    }
    else
    {
        throw std::runtime_error("Cannot find toolset " + name);
    }

    toolset->SetParameter("name", name);
    return toolset;
}

std::unique_ptr<Toolset> ParseToolset(const std::string &name)
{
    std::unique_ptr<Toolset> toolset;
    std::ifstream in(name);
    while(!in.eof() && in.good())
    {
        std::string line;
        std::getline(in, line);
        line = line.substr(0, line.find_first_of('#'));
        size_t colon = line.find_first_of(':');
        if(colon == line.npos)
            continue;
        std::string key = line.substr(0, colon);
        colon++;
        while(line[colon] == ' ')
            colon++;
        std::string value = line.substr(colon);
        while(!value.empty() && value[value.size() - 1] == ' ')
            value.resize(value.size() - 1);
        if(key == "template") {
            toolset = GetToolsetByName(value);
            auto hash = sha512(name);
            for (size_t n = 0; n < 64; n++) {
                toolset->hash[n] ^= hash[n];
            }
        }
        else if(!toolset)
        {
            throw std::runtime_error("No template toolset found before parameters while loading " + name);
        }
        else
        {
            toolset->SetParameter(key, value);
        }
    }
    return toolset;
}

