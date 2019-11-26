#include "Toolset.h"

#include "Component.h"
#include "fw/dotted.h"

#include <fstream>
#include <fw/filesystem.hpp>
#include <string>

std::string Toolset::getNameFor(const Component &component)
{
    return component.GetName();
}

std::unique_ptr<Toolset> ParseToolset(const std::string &name);

std::unique_ptr<Toolset> GetToolsetByName(const std::string &name)
{
    std::unique_ptr<Toolset> toolset;
    if(fs::is_regular_file("toolsets/" + name + ".toolset") && name.substr(0, 10) != "__builtin_")
    {
        toolset = ParseToolset("toolsets/" + name + ".toolset");
    }
    else if(name == "windows" || name == "msvc" || name == "__builtin_msvc")
    {
        toolset = std::make_unique<MsvcToolset>();
    }
    else if(name == "apple" || name == "osx" || name == "clang" || name == "__builtin_clang")
    {
        toolset = std::make_unique<ClangToolset>();
    }
    else if(name == "linux" || name == "gcc" || name == "__builtin_gcc")
    {
        toolset = std::make_unique<GccToolset>();
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
        if(key == "template")
            toolset = GetToolsetByName(value);
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

