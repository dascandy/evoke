#include "Component.h"
#include "Configuration.h"
#include "File.h"
#include "PendingCommand.h"
#include "Project.h"
#include "Toolset.h"
#include "boost/algorithm/string/classification.hpp"
#include "dotted.h"
#include "utils.h"

#include <algorithm>
#include <stack>
#include <boost/algorithm/string/split.hpp>


static const std::string compiler = "g++";
static const std::string archiver = "ar";

static std::string getLibNameFor(Component &component)
{
    return "lib" + as_dotted(component.root.string()) + ".a";
}

static std::string getExeNameFor(Component &component)
{
    if(component.root.string() != ".")
    {
        return as_dotted(component.root.string());
    }
    return filesystem::canonical(component.root).filename().string();
}

void UbuntuToolset::CreateCommandsFor(Project &project)
{
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
            if(!project.IsCompilationUnit(f->path.extension().string()))
                continue;
            filesystem::path outputFile = std::string("obj") / outputFolder / (f->path.string().substr(component.root.string().size()) + ".o");
            File *of = project.CreateFile(component, outputFile);
            PendingCommand *pc = new PendingCommand(compiler + " -c " + Configuration::Get().compileFlags + " -o " + outputFile.string() + " " + f->path.string() + includes);
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
            PendingCommand *pc;
            if(component.type == "library")
            {
                outputFile = "lib/" + getLibNameFor(component);
                command = archiver + " rcs " + outputFile.string();
                for(auto &file : objects)
                {
                    command += " " + file->path.string();
                }
                pc = new PendingCommand(command);
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
                        if(d[index]->isHeaderOnly())
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
                    if(d.size() == 1 || (d.size() == 2 && (d[0] == &component || d[1] == &component)))
                    {
                        if(d[0] != &component)
                        {
                            command += " -l" + d[0]->root.string();
                        }
                        else if(d.size() == 2)
                        {
                            command += " -l" + d[1]->root.string();
                        }
                    }
                    else
                    {
                        command += " -Wl,--start-group";
                        for(auto &c : d)
                        {
                            if(c != &component)
                            {
                                command += " -l" + c->root.string();
                            }
                        }
                        command += " -Wl,--end-group";
                    }
                }
                pc = new PendingCommand(command);
                for(auto &d : linkDeps)
                {
                    for(auto &c : d)
                    {
                        if(c != &component)
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
                pc = new PendingCommand(command);
                outputFile += ".log";
                pc->AddInput(libraryFile);
                pc->AddOutput(project.CreateFile(component, outputFile.string()));
                pc->Check();
                component.commands.push_back(pc);
            }
        }
    }
}

GlobalOptions UbuntuToolset::ParseGeneralOptions(const std::string &options)
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
