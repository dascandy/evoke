#include "Component.h"
#include "File.h"
#include "PendingCommand.h"
#include "Project.h"
#include "Toolset.h"
#include "fw/dotted.h"

#include <algorithm>
#include <stack>

MsvcToolset::MsvcToolset()
{
    SetParameter("name", "msvc");
    SetParameter("compiler", "cl.exe");
    SetParameter("linker", "link.exe");
    SetParameter("archiver", "lib.exe");
}

std::string MsvcToolset::getBmiNameFor(const File &file)
{
    return file.path.generic_string() + ".bmi";
}

std::string MsvcToolset::getObjNameFor(const File &file)
{
    return file.path.generic_string() + ".obj";
}

std::string MsvcToolset::getLibNameFor(const Component &component)
{
    return "lib" + getNameFor(component) + ".lib";
}

std::string MsvcToolset::getExeNameFor(const Component &component)
{
    return getNameFor(component) + ".exe";
}

std::string MsvcToolset::getPrecompileCommand(const std::string &program, const std::string &outputFile, const File *inputFile, const std::set<std::string> &includes, bool hasModules)
{
    std::string command = program + " /nologo /c /EHsc";
    if(hasModules)
    {
        command += " /experimental:module /std:c++20 /MD /permissive-";
    }
    command += " /Fo" + outputFile + " " + inputFile->path.generic_string();
    for(auto &i : includes)
        command += " /I" + i;
    return command;
}

std::string MsvcToolset::getCompileCommand(const std::string &program, const std::string &outputFile, const File *inputFile, const std::set<std::string> &includes, bool hasModules)
{
    std::string command = program + " /nologo /c /EHsc";
    if(hasModules)
    {
        command += " /experimental:module /std:c++20 /MD /permissive-";
    }
    command += " /Fo" + outputFile + " " + inputFile->path.generic_string();
    for(auto &i : includes)
        command += " /I" + i;
    return command;
}

std::string MsvcToolset::getArchiverCommand(const std::string &program, const std::string &outputFile, const std::vector<File *> inputs)
{
    std::string command = program + " /OUT:" + outputFile;
    for(auto &file : inputs)
    {
        command += " " + file->path.generic_string();
    }
    return command;
}

std::string MsvcToolset::getLinkerCommand(const std::string &program, const std::string &outputFile, const std::vector<File *> objects, std::vector<std::vector<Component *>> linkDeps)
{
    std::string command = program + " /OUT:" + outputFile;
    for(auto &file : objects)
    {
        command += " " + file->path.string();
    }
    command += " /LIBPATH:build/" + GetParameter("name") + "/lib";
    for(auto &d : linkDeps)
        for(auto &c : d)
        {
            command += " " + getLibNameFor(*c);
        }
    return command;
}

std::string MsvcToolset::getUnittestCommand(const std::string &program)
{
    return program;
}
