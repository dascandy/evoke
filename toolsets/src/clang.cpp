#include "Component.h"
#include "Configuration.h"
#include "File.h"
#include "PendingCommand.h"
#include "Project.h"
#include "Toolset.h"
#include "boost/algorithm/string/classification.hpp"
#include "dotted.h"
#include "globaloptions.h"

#include <algorithm>
#include <boost/algorithm/string/split.hpp>
#include <stack>

ClangToolset::ClangToolset()
{
    compiler = "clang++";
    linker = "clang++";
    archiver = "ar";
}

void ClangToolset::SetParameter(const std::string &key, const std::string &value)
{
    if(key == "compiler")
        compiler = value;
    else if(key == "linker")
        linker = value;
    else if(key == "archiver")
        archiver = value;
    else
        throw std::runtime_error("Invalid parameter for Clang toolchain: " + key);
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

std::string ClangToolset::getUnityCommand(const std::string &program, const std::string &compileFlags, const std::string &outputFile, const File *inputFile, const std::set<std::string> &includes, std::vector<std::vector<Component *>> linkDeps)
{
    std::string command = program + " -c " + compileFlags + " -o " + outputFile + " " + inputFile->path.generic_string();
    for(auto &i : includes)
        command += " -I" + i;
    for(auto d : linkDeps)
    {
        for(auto &c : d)
        {
            command += " -l" + c->root.string();
        }
    }
    return command;
}

std::string ClangToolset::getPrecompileCommand(const std::string &program, const std::string &compileFlags, const std::string &outputFile, const File *inputFile, const std::set<std::string> &includes, bool hasModules)
{
    std::string command = program + " -fmodules-ts --precompile -x c++-module " + compileFlags + " -o " + outputFile + " " + inputFile->path.generic_string();
    if(hasModules)
        command += " -fprebuilt-module-path=modules";
    for(auto &i : includes)
        command += " -I" + i;
    return command;
}

std::string ClangToolset::getCompileCommand(const std::string &program, const std::string &compileFlags, const std::string &outputFile, const File *inputFile, const std::set<std::string> &includes, bool hasModules)
{
    std::string command = program + " -c " + compileFlags + " -o " + outputFile + " " + inputFile->path.generic_string();
    if(hasModules)
        command += " -fmodules-ts -fprebuilt-module-path=modules";
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
    std::string command = program + " -pthread -o " + outputFile;
    for(auto &file : objects)
    {
        command += " " + file->path.string();
    }
    command += " -Llib";
    for(auto d : linkDeps)
    {
        if(d.size() == 1)
        {
            command += " -l" + d.front()->root.string();
        }
        else
        {
            for(auto &c : d)
            {
                command += " -l" + c->root.string();
            }
        }
    }
    return command;
}

std::string ClangToolset::getUnittestCommand(const std::string &program)
{
    return "./" + program;
}

GlobalOptions ClangToolset::ParseGeneralOptions(const std::string &options)
{
    GlobalOptions opts;
    //std::vector<std::string> parts = splitWithQuotes(options);
    std::vector<std::string> parts;
    boost::split(parts, options, boost::algorithm::is_any_of(" \t"), boost::algorithm::token_compress_on);

    for(const auto &opt : parts)
    {
        if(opt.find("-I") == 0)
        {
            opts.include.emplace_back(opt.substr(2));
        }
        else if(opt.find("-L") == 0)
        {
            opts.link.emplace_back(opt);
        }
        else if(opt == "-pthread")
        {
            opts.compile.emplace_back("-pthread");
            opts.link.emplace_back("-lpthread");
        }
        else
        {
            opts.compile.emplace_back(opt);
        }
    }
    return opts;
}
