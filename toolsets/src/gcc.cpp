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
#include <unordered_set>

std::string GccToolset::getLibNameFor(Component &component)
{
    return "lib" + getNameFor(component) + ".a";
}

std::string GccToolset::getExeNameFor(Component &component)
{
    return getNameFor(component);
}

GccToolset::GccToolset() 
: compiler("g++")
, linker("g++")
, archiver("ar")
{
}

void GccToolset::SetParameter(const std::string& key, const std::string& value) {
  if (key == "compiler") compiler = value;
  else if (key == "linker") linker = value;
  else if (key == "archiver") archiver = value;
  else throw std::runtime_error("Invalid parameter for GCC toolchain: " + key);
}

void GccToolset::CreateCommandsForUnity(Project &project)
{
    for(auto &p : project.components)
    {
        auto &component = p.second;
        if (component.type == "library") continue;

        std::vector<std::vector<Component *>> allDeps = GetTransitiveAllDeps(component);
        std::vector<Component*> deps;
        std::vector<File*> files;
        std::string linkline;
        std::unordered_set<std::string> includes;
        filesystem::create_directories("unity");
        filesystem::path outputFile = std::string("unity") + "/" + getExeNameFor(component) + ".cpp";
        File* of = project.CreateFile(component, outputFile);
        std::ofstream out(outputFile.generic_string());
        for (auto& v : allDeps) for (auto& c : v) {
            if (c->isBinary) {
                linkline += " -l" + getNameFor(*c);
            } else {
                for(auto &d : getIncludePathsFor(component))
                {
                    includes.insert(d);
                }
                for (auto& f : c->files) {
                    files.push_back(f);
                    if(File::isTranslationUnit(f->path))
                    {
                        out << "#include \"../" + f->path.generic_string() << "\"\n";
                    }
                }
            }
        }

        filesystem::path exeFile = "bin/" + getExeNameFor(component);
        std::string includeString;
        for (auto& i : includes) {
            includeString += " -I" + i;
        }
        std::shared_ptr<PendingCommand> pc = std::make_shared<PendingCommand>(compiler + " " + Configuration::Get().compileFlags + " " + includeString + " -pthread -o " + exeFile.generic_string() + " " + outputFile.generic_string() + linkline);

        File *executable = project.CreateFile(component, exeFile);
        pc->AddOutput(executable);
        pc->AddInput(of);
        for (auto& f : files)
            pc->AddInput(f);
        pc->Check();
        component.commands.push_back(pc);
        if(component.type == "unittest")
        {
            pc = std::make_shared<PendingCommand>(exeFile.string());
            exeFile += ".log";
            pc->AddInput(executable);
            pc->AddOutput(project.CreateFile(component, exeFile.string()));
            pc->Check();
            component.commands.push_back(pc);
        }
    }
}

void GccToolset::CreateCommandsFor(Project &project)
{
    for(auto &p : project.components) 
    {
        // Precompile all modules (with dependencies on BMIs, for the graph)

    }
    for(auto &p : project.components)
    {
        auto &component = p.second;
        std::string includes;
        for(auto &d : getIncludePathsFor(component))
        {
            includes += " -I" + d;
        }

        // TODO: modules: -fmodules-ts --precompile  -fmodules-cache-path=<directory>-fprebuilt-module-path=<directory>
        filesystem::path outputFolder = component.root;
        std::vector<File *> objects;
        for(auto &f : component.files)
        {
            if(!File::isTranslationUnit(f->path))
                continue;
            filesystem::path outputFile = std::string("obj") / outputFolder / (f->path.string().substr(component.root.string().size()) + ".o");
            File *of = project.CreateFile(component, outputFile);
            std::shared_ptr<PendingCommand> pc = std::make_shared<PendingCommand>(compiler + " -c " + Configuration::Get().compileFlags + " -o " + outputFile.string() + " " + f->path.string() + includes);
            objects.push_back(of);
            pc->AddOutput(of);
            std::unordered_set<File *> d;
            std::stack<File *> deps;
            deps.push(f);
            size_t index = 0;
            while(!deps.empty())
            {
                File *dep = deps.top();
                deps.pop();
                pc->AddInput(dep);
                for(File *input : dep->dependencies)
                    if(d.insert(input).second)
                        deps.push(input);
                index++;
            }
            pc->Check();
            component.commands.push_back(pc);
        }
        if(!objects.empty())
        {
            std::string command;
            filesystem::path outputFile;
            std::shared_ptr<PendingCommand> pc;
            if(component.type == "library")
            {
                outputFile = "lib/" + getLibNameFor(component);
                command = archiver + " rcs " + outputFile.string();
                for(auto &file : objects)
                {
                    command += " " + file->path.string();
                }
                pc = std::make_shared<PendingCommand>(command);
            }
            else
            {
                outputFile = "bin/" + getExeNameFor(component);
                command = compiler + " -pthread -o " + outputFile.string();

                for(auto &file : objects)
                {
                    command += " " + file->path.string();
                }
                command += " -Llib";
                std::vector<std::vector<Component *>> linkDeps = GetTransitiveAllDeps(component);
                std::reverse(linkDeps.begin(), linkDeps.end());
                for(auto d : linkDeps)
                {
                    size_t index = 0;
                    while(index < d.size())
                    {
                        if(d[index] == &component || d[index]->isHeaderOnly())
                        {
                            d[index] = d.back();
                            d.pop_back();
                        }
                        else
                        {
                            ++index;
                        }
                    }
                    if(d.empty())
                        continue;
                    if(d.size() == 1)
                    {
                        command += " -l" + d[0]->root.string();
                    }
                    else
                    {
                        command += " -Wl,--start-group";
                        for(auto &c : d)
                        {
                            if(c != &component && !c->isHeaderOnly())
                            {
                                command += " -l" + c->root.string();
                            }
                        }
                        command += " -Wl,--end-group";
                    }
                }
                pc = std::make_shared<PendingCommand>(command);
                for(auto &d : linkDeps)
                {
                    for(auto &c : d)
                    {
                        if(c != &component && !c->isHeaderOnly())
                        {
                            pc->AddInput(project.CreateFile(*c, "lib/" + getLibNameFor(*c)));
                        }
                    }
                }
            }
            File *libraryFile = project.CreateFile(component, outputFile);
            pc->AddOutput(libraryFile);
            for(auto &file : objects)
            {
                pc->AddInput(file);
            }
            pc->Check();
            component.commands.push_back(pc);
            if(component.type == "unittest")
            {
                command = outputFile.string();
                pc = std::make_shared<PendingCommand>(command);
                outputFile += ".log";
                pc->AddInput(libraryFile);
                pc->AddOutput(project.CreateFile(component, outputFile.string()));
                pc->Check();
                component.commands.push_back(pc);
            }
        }
    }
}

GlobalOptions GccToolset::ParseGeneralOptions(const std::string &options)
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
