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

static std::unordered_set<File *> GetDependencies(File *file, std::unordered_map<std::string, File *> &moduleMap)
{
    std::unordered_set<File *> d;
    std::stack<File *> deps;
    deps.push(file);
    size_t index = 0;
    while(!deps.empty())
    {
        File *dep = deps.top();
        deps.pop();
        for(auto &input : dep->dependencies)
            if(d.insert(input.second).second)
                deps.push(input.second);
        for(auto &p : dep->modImports) {
            auto it = moduleMap.find(p.first);
            if (it != moduleMap.end()) deps.push(it->second);
        }
        for(auto &p : dep->imports) {
            auto it = moduleMap.find(p.first);
            if (it != moduleMap.end()) deps.push(it->second);
        }
        index++;
    }
    return d;
}

void GenericToolset::CreateCommandsForUnity(Project &project)
{
    for(auto &p : project.components)
    {
        auto &component = p.second;
        if(component.type == "library")
            continue;

        std::vector<std::vector<Component *>> allDeps = GetTransitiveAllDeps(component);
        std::vector<Component *> deps;
        std::vector<File *> files;
        std::string linkline;
        std::set<std::string> includes;
        filesystem::create_directories("unity");
        filesystem::path outputFile = std::string("unity") + "/" + getExeNameFor(component) + ".cpp";
        File *of = project.CreateFile(component, outputFile);
        std::ofstream out(outputFile.generic_string());
        for(auto &v : allDeps)
            for(auto &c : v)
            {
                if(c->isBinary)
                {
                    linkline += " -l" + getNameFor(*c);
                }
                else
                {
                    for(auto &d : getIncludePathsFor(component))
                    {
                        includes.insert(d);
                    }
                    for(auto &f : c->files)
                    {
                        files.push_back(f);
                        if(File::isTranslationUnit(f->path))
                        {
                            out << "#include \"../" + f->path.generic_string() << "\"\n";
                        }
                    }
                }
            }
        // TODO: missing header dependencies

        std::vector<std::vector<Component *>> inputLinkDeps = GetTransitiveAllDeps(component);
        std::reverse(inputLinkDeps.begin(), inputLinkDeps.end());
        std::vector<std::vector<Component *>> linkDeps;
        for(auto &in : inputLinkDeps)
        {
            size_t offs = 0;
            while(offs < in.size())
            {
                if(in[offs] == &component || in[offs]->isHeaderOnly())
                {
                    in[offs] = in.back();
                    in.pop_back();
                }
                else
                {
                    ++offs;
                }
            }
            if(!in.empty())
                linkDeps.push_back(std::move(in));
        }

        filesystem::path exeFile = "bin/" + getExeNameFor(component);
        std::shared_ptr<PendingCommand> pc = std::make_shared<PendingCommand>(getUnityCommand(compiler, Configuration::Get().compileFlags, outputFile.generic_string(), of, includes, linkDeps));

        File *executable = project.CreateFile(component, exeFile);
        pc->AddOutput(executable);
        pc->AddInput(of);
        for(auto &f : files)
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

void GenericToolset::CreateCommandsFor(Project &project)
{
    std::unordered_map<std::string, File *> moduleMap;
    std::set<File *> toPrecompile;
    for(auto &[name, component] : project.components)
    {
        for(auto &f : component.files)
        {
            if(!f->moduleExported)
                continue;

            File *ofile = project.CreateFile(component, "modules/" + getBmiNameFor(*f));
            moduleMap.insert(std::make_pair(f->moduleName, ofile));
            toPrecompile.insert(f);
            for(auto &import : f->modImports)
            {
                File *ofile = project.CreateFile(component, "modules/" + getBmiNameFor(*import.second));
                moduleMap.insert(std::make_pair(import.first, ofile));
                toPrecompile.insert(f);
            }
        }
    }
    if(!moduleMap.empty())
    {
        std::ofstream os("module.map");
        for(auto &p : moduleMap)
        {
            os << p.first << "=" << p.second->path.generic_string() << "\n";
        }
    }
    for(auto &f : toPrecompile)
    {
        auto includes = getIncludePathsFor(f->component);
        File *ofile = project.CreateFile(f->component, "modules/" + getBmiNameFor(*f));
        std::shared_ptr<PendingCommand> pc = std::make_shared<PendingCommand>(getPrecompileCommand(compiler, Configuration::Get().compileFlags, ofile->path.generic_string(), f, includes, true));
        pc->AddOutput(ofile);
        for(auto &d : GetDependencies(f, moduleMap))
        {
            pc->AddInput(d);
        }
        pc->Check();
        f->component.commands.push_back(pc);
    }
    for(auto &p : project.components)
    {
        auto &component = p.second;
        auto includes = getIncludePathsFor(component);
        std::vector<File *> objects;
        for(auto &f : component.files)
        {
            if(!File::isTranslationUnit(f->path))
                continue;
            filesystem::path outputFile = "obj/" + getObjNameFor(*f);
            File *of = project.CreateFile(component, outputFile);
            std::shared_ptr<PendingCommand> pc = std::make_shared<PendingCommand>(getCompileCommand(compiler, Configuration::Get().compileFlags, outputFile.generic_string(), f, includes, !f->moduleName.empty() || !f->imports.empty() || !f->modImports.empty()));
            objects.push_back(of);
            pc->AddOutput(of);
            pc->AddInput(f);
            for(auto &d : GetDependencies(f, moduleMap))
            {
                pc->AddInput(d);
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
                command = getArchiverCommand(archiver, outputFile.generic_string(), objects);
                pc = std::make_shared<PendingCommand>(command);
            }
            else
            {
                outputFile = "bin/" + getExeNameFor(component);
                std::vector<std::vector<Component *>> inputLinkDeps = GetTransitiveAllDeps(component);
                std::reverse(inputLinkDeps.begin(), inputLinkDeps.end());
                std::vector<std::vector<Component *>> linkDeps;
                for(auto &in : inputLinkDeps)
                {
                    size_t offs = 0;
                    while(offs < in.size())
                    {
                        if(in[offs] == &component || in[offs]->isHeaderOnly())
                        {
                            in[offs] = in.back();
                            in.pop_back();
                        }
                        else
                        {
                            ++offs;
                        }
                    }
                    if(!in.empty())
                        linkDeps.push_back(std::move(in));
                }
                command = getLinkerCommand(linker, outputFile.generic_string(), objects, linkDeps);
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
                pc = std::make_shared<PendingCommand>(getUnittestCommand(command));
                outputFile += ".log";
                pc->AddInput(libraryFile);
                pc->AddOutput(project.CreateFile(component, outputFile.string()));
                pc->Check();
                component.commands.push_back(pc);
            }
        }
    }
}
