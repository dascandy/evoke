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

ClangToolset::ClangToolset()
{
    SetParameter("name", "clang");
    SetParameter("compiler", "clang++");
    SetParameter("linker", "clang++");
    SetParameter("archiver", "ar");
}

std::string ClangToolset::getBmiNameFor(const File &file)
{
    return file.moduleName + ".pcm";
    //return file.path.generic_string() + ".bmi";
}

std::string ClangToolset::getObjNameFor(const File &file)
{
    return file.path.generic_string() + ".o";
}

std::string ClangToolset::getLibNameFor(const Component &component)
{
    return "lib" + getNameFor(component) + ".a";
}

std::string ClangToolset::getExeNameFor(const Component &component)
{
    return getNameFor(component);
}

std::string ClangToolset::getPrecompileCommand(const std::string &program, const std::string &outputFile, const File *inputFile, const std::set<std::string> &includes, bool hasModules)
{
    std::string command = program + " -fmodules --precompile -x c++-module -o " + outputFile + " " + inputFile->path.generic_string();
    if(hasModules)
        command += " -fprebuilt-module-path=build/" + GetParameter("name") + "/modules";
    for(auto &i : includes)
        command += " -I" + i;
    return command;
}

std::string ClangToolset::getCompileCommand(const std::string &program, const std::string &outputFile, const File *inputFile, const std::set<std::string> &includes, bool hasModules)
{
    std::string command = program + " -c -o " + outputFile + " " + inputFile->path.generic_string();
    if(hasModules)
        command += " -fmodules -fprebuilt-module-path=build/" + GetParameter("name") + "/modules";
    for(auto &i : includes)
        command += " -I" + i;
    return command;
}

std::string ClangToolset::getArchiverCommand(const std::string &program, const std::string &outputFile, const std::vector<File *> inputs)
{
    std::string command = program + " rcs " + outputFile;
    for(auto &file : inputs)
    {
        command += " " + file->path.generic_string();
    }
    return command;
}

std::string ClangToolset::getLinkerCommand(const std::string &program, const std::string &outputFile, const std::vector<File *> objects, std::vector<std::vector<Component *>> linkDeps)
{
    std::string command = program + " -o " + outputFile;
    for(auto &file : objects)
    {
        command += " " + file->path.string();
    }
    command += " -Lbuild/" + GetParameter("name") + "/lib";
    for(auto d : linkDeps)
    {
        for(auto &c : d)
        {
            command += " -l" + c->GetName();
        }
    }
    return command;
}

std::string ClangToolset::getUnittestCommand(const std::string &program)
{
    return program;
}

