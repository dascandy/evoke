#include "Component.h"
#include "File.h"
#include "PendingCommand.h"
#include "Project.h"
#include "Toolset.h"
#include "fw/dotted.h"

#include <algorithm>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <stack>
#include <unordered_set>

GccToolset::GccToolset()
{
    SetParameter("name", "gcc");
    SetParameter("compiler", "g++ -fdiagnostics-color=always");
    SetParameter("linker", "g++");
    SetParameter("archiver", "ar");
}

std::string GccToolset::getBmiNameFor(const File &file)
{
    return file.path.generic_string() + ".bmi";
}

std::string GccToolset::getObjNameFor(const File &file)
{
    return file.path.generic_string() + ".o";
}

std::string GccToolset::getLibNameFor(const Component &component)
{
    return "lib" + getNameFor(component) + ".a";
}

std::string GccToolset::getExeNameFor(const Component &component)
{
#if defined(_WIN32)
    return getNameFor(component) + ".exe";
#else
    return getNameFor(component);
#endif
}

std::string GccToolset::getUnityCommand(const std::string &program, const std::string &outputFile, const File *inputFile, const std::set<std::string> &includes, std::vector<std::vector<Component *>> linkDeps)
{
    std::string command = program + " -c -o " + outputFile + " " + inputFile->path.generic_string();
    for(auto &i : includes)
        command += " -I" + i;
    for(auto d : linkDeps)
    {
        if(d.size() == 1)
        {
            command += " -l" + d.front()->root.string();
        }
        else
        {
            command += " -Wl,--start-group";
            for(auto &c : d)
            {
                command += " -l" + getNameFor(*c);
            }
            command += " -Wl,--end-group";
        }
    }
    return command;
}

std::string GccToolset::getPrecompileCommand(const std::string &program, const std::string &outputFile, const File *inputFile, const std::set<std::string> &includes, bool hasModules)
{
    std::string command = program + " -fmodules-ts -c -o " + outputFile + " " + inputFile->path.generic_string();
    if(!inputFile->moduleExported)
        command += " -fmodule-legacy";
    if(hasModules)
        command += " -fmodule-mapper=module.map";
    for(auto &i : includes)
        command += " -I" + i;
    return command;
}

std::string GccToolset::getCompileCommand(const std::string &program, const std::string &outputFile, const File *inputFile, const std::set<std::string> &includes, bool hasModules)
{
    std::string command = program + " -c -o " + outputFile + " " + inputFile->path.generic_string();
    if(hasModules)
        command += " -fmodules-ts -fmodule-mapper=module.map";

    for(auto &i : includes)
        command += " -I" + i;
    return command;
}

std::string GccToolset::getArchiverCommand(const std::string &program, const std::string &outputFile, const std::vector<File *> inputs)
{
    std::string command = program + " rcs " + outputFile;
    for(auto &file : inputs)
    {
        command += " " + file->path.generic_string();
    }
    return command;
}

std::string GccToolset::getLinkerCommand(const std::string &program, const std::string &outputFile, const std::vector<File *> objects, std::vector<std::vector<Component *>> linkDeps)
{
    std::string command = program + " -pthread -o " + outputFile;
    for(auto &file : objects)
    {
        command += " " + file->path.string();
    }
    command += " -Lbuild/" + GetParameter("name") + "/lib";
    for(auto d : linkDeps)
    {
        if(d.size() == 1)
        {
            command += " -l" + getNameFor(*d.front());
        }
        else
        {
            command += " -Wl,--start-group";
            for(auto &c : d)
            {
                command += " -l" + getNameFor(*c);
            }
            command += " -Wl,--end-group";
        }
    }
    return command;
}

std::string GccToolset::getUnittestCommand(const std::string &program)
{
    return program;
}

