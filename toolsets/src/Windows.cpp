#include "Component.h"
#include "Configuration.h"
#include "File.h"
#include "PendingCommand.h"
#include "Project.h"
#include "Toolset.h"
#include "dotted.h"

#include <algorithm>
#include <stack>

static const std::string compiler = "cl.exe";
static const std::string linker = "link.exe";
static const std::string archiver = "lib.exe";
//https://blogs.msdn.microsoft.com/vcblog/2015/12/03/c-modules-in-vs-2015-update-1/

// Enable modules support for MSVC
//"/experimental:module /module:stdIfcDir \"$(VC_IFCPath)\" /module:search obj/modules/"

// Tell MSVC to act like a sane compiler (use the latest C++ version in standards conforming mode, use C++ exceptions as specified, and link to the C/C++ runtime)
//"/std:c++latest /permissive- /EHsc /MD"

static std::string getLibNameFor(Component &component)
{
    return "lib" + as_dotted(component.root.string()) + ".lib";
}

static std::string getExeNameFor(Component &component)
{
    if(component.root.string() != ".")
    {
        return as_dotted(component.root.string()) + ".exe";
    }
    return filesystem::weakly_canonical(component.root).filename().string() + ".exe";
}

void WindowsToolset::CreateCommandsFor(Project &project)
{
    for(auto &p : project.components)
    {
        auto &component = p.second;
        std::string includes;
        for(auto &d : getIncludePathsFor(component))
        {
            includes += " /I" + filesystem::relative(d).string();
        }

        // TODO: modules: -fmodules-ts --precompile  -fmodules-cache-path=<directory>-fprebuilt-module-path=<directory>
        filesystem::path outputFolder = component.root;
        std::vector<File *> objects;
        for(auto &f : component.files)
        {
            if(!project.IsCompilationUnit(f->path.extension().string()))
                continue;
            filesystem::path temp = (f->path.string().substr(component.root.string().size()) + ".obj");
            filesystem::path outputFile = std::string("obj") / outputFolder / temp;
            File *of = project.CreateFile(component, outputFile);
            PendingCommand *pc = new PendingCommand(compiler + " /c /EHsc " + Configuration::Get().compileFlags + includes + " /Fo" + filesystem::weakly_canonical(outputFile).string() + " " + filesystem::weakly_canonical(f->path).string());
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
                outputFile = "lib\\" + getLibNameFor(component);
                command = archiver + " " + filesystem::weakly_canonical(outputFile).string();
                for(auto &file : objects)
                {
                    command += " " + file->path.string();
                }
                pc = new PendingCommand(command);
            }
            else
            {
                outputFile = "bin\\" + getExeNameFor(component);
                command = linker + " /OUT:" + outputFile.string();

                for(auto &file : objects)
                {
                    command += " " + file->path.string();
                }
                if(filesystem::exists("lib"))
                {
                    command += " /LIBPATH:lib";
                }
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
                            command += " " + d[0]->root.string();
                        }
                        else if(d.size() == 2)
                        {
                            command += " " + d[1]->root.string();
                        }
                    }
                    else
                    {
                        for(auto &c : d)
                        {
                            if(c != &component)
                            {
                                command += " " + c->root.string();
                            }
                        }
                    }
                }
                pc = new PendingCommand(command);
                for(auto &d : linkDeps)
                {
                    for(auto &c : d)
                    {
                        if(c != &component)
                        {
                            pc->AddInput(project.CreateFile(*c, "lib\\" + getLibNameFor(*c)));
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
