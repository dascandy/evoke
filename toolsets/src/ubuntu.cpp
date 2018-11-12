#include "Component.h"
#include "File.h"
#include "PendingCommand.h"
#include "Project.h"
#include "Toolset.h"

#include <algorithm>

std::string as_dotted(std::string str)
{
    std::replace(str.begin(), str.end(), '/', '.');
    return str;
}

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
    return boost::filesystem::canonical(component.root).filename().string();
}

void UbuntuToolset::CreateCommandsFor(Project &project)
{
    for(auto &[name, component] : project.components)
    {
        std::string includes;
        for(auto &d : getIncludePathsFor(component))
        {
            includes += " -I" + d;
        }

        // TODO: modules: -fmodules-ts --precompile  -fmodules-cache-path=<directory>-fprebuilt-module-path=<directory>

        boost::filesystem::path outputFolder = component.root;
        std::vector<File *> objects;
        for(auto &f : component.files)
        {
            if(!project.IsCompilationUnit(f->path.extension().string()))
                continue;
            boost::filesystem::path outputFile = std::string("obj") / outputFolder / (f->path.string().substr(component.root.string().size()) + ".o");
            File *of = project.CreateFile(component, outputFile);
            PendingCommand *pc = new PendingCommand("g++ -c -std=c++17 -o " + outputFile.string() + " " + f->path.string() + includes);
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
            boost::filesystem::path outputFile;
            PendingCommand *pc;
            if(component.type == "library")
            {
                outputFile = "lib/" + getLibNameFor(component);
                command = "ar rcs " + outputFile.string();
                for(auto &file : objects)
                {
                    command += " " + file->path.string();
                }
                pc = new PendingCommand(command);
            }
            else
            {
                outputFile = "bin/" + getExeNameFor(component);
                command = "g++ -pthread -o " + outputFile.string();

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
