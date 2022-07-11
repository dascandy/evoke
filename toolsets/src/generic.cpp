#include "Component.h"
#include "File.h"
#include "PendingCommand.h"
#include "Project.h"
#include "Toolset.h"
#include "fw/dotted.h"
#include <regex>
#include <algorithm>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <fstream>
#include <stack>
#include <string>

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
        for(auto &p : dep->modImports)
        {
            auto it = moduleMap.find(p.first);
            if(it != moduleMap.end()) {
                if(d.insert(it->second).second)
                    deps.push(it->second);
            }
        }
        for(auto &p : dep->imports)
        {
            auto it = moduleMap.find(p.first);
            if(it != moduleMap.end()) {
                if(d.insert(it->second).second)
                    deps.push(it->second);
            }
        }
        index++;
    }
    return d;
}

void GenericToolset::SetParameter(const std::string &key, const std::string &value)
{
    parameters[key] = value;
}

std::string GenericToolset::GetParameter(const std::string& key) {
  auto it = parameters.find(key);
  if (it == parameters.end()) throw std::runtime_error("no such parameter: " + key);
  std::string baseRV = it->second;
  std::regex re("\\$(\\([^)]*\\))");
  std::smatch matches;
  while (std::regex_search(baseRV, matches, re)) {
    baseRV = std::string(matches.prefix()) + GetParameter(matches[1]) + std::string(matches.suffix());
  }
  return baseRV;
}

GenericToolset::GenericToolset() {
  SetParameter("build-executable", "true");
  SetParameter("build-unittest", "true");
  SetParameter("build-fuzzer", "false");
  SetParameter("run-unittest", "true");
}

std::string GenericToolset::GetCompilerFor(std::string extension) {
  auto it = parameters.find("compiler-" + extension.substr(1));
  if (it != parameters.end()) {
    return GetParameter("compiler-" + extension.substr(1));
  } else {
    return GetParameter("compiler");
  }
}

void GenericToolset::CreateCommandsFor(Project &project, const std::vector<std::string>& targets)
{
    std::unordered_map<std::string, File *> moduleMap;
    std::set<File *> toPrecompile;
    std::unordered_map<File*, File*> precompileds;
    std::set<Component*> componentsToCompile;
    if (targets.empty()) {
        for (auto& [name, component] : project.components) {
            componentsToCompile.insert(&component);
        }
    } else {
        std::vector<Component*> components;
        for (const auto& target : targets) {
            auto it = project.components.find(target);
            if (it == project.components.end()) {
                puts(("Cannot find component " + target).c_str());
                continue;
            }
            components.push_back(&it->second);
        }
        while (not components.empty()) {
            Component* component = components.back();
            components.pop_back();
            if (componentsToCompile.insert(component).second) {
                components.insert(components.end(), component->pubDeps.begin(), component->pubDeps.end());
                components.insert(components.end(), component->privDeps.begin(), component->privDeps.end());
            }
        }
    }

    for(auto &component : componentsToCompile)
    {
        for(auto &f : component->files)
        {
            if(!f->moduleExported)
                continue;

            File *ofile = project.CreateFile(*component, "build/" + GetParameter("name") + "/modules/" + getBmiNameFor(*f));
            moduleMap.insert(std::make_pair(f->moduleName, ofile));
            toPrecompile.insert(f);
            precompileds.insert(std::make_pair(f, ofile));
            for(auto &import : f->modImports)
            {
                File *ofile = project.CreateFile(*component, "build/" + GetParameter("name") + "/modules/" + getBmiNameFor(*import.second));
                moduleMap.insert(std::make_pair(import.first, ofile));
                toPrecompile.insert(f);
            }
        }
    }
    if(!moduleMap.empty())
    {
        std::ofstream os("build/" + GetParameter("name") + "/module.map");
        for(auto &p : moduleMap)
        {
            os << p.first << "=" << p.second->path.generic_string() << "\n";
        }
    }
    for(auto &f : toPrecompile)
    {
        auto includes = getIncludePathsFor(f->component);
        File *ofile = project.CreateFile(f->component, "build/" + GetParameter("name") + "/modules/" + getBmiNameFor(*f));
        std::shared_ptr<PendingCommand> pc = std::make_shared<PendingCommand>(hash, getPrecompileCommand(GetCompilerFor(f->path.extension().string()), ofile->path.generic_string(), f, includes, true));
        pc->AddOutput(ofile);
        pc->AddInput(f);
        for(auto &d : GetDependencies(f, moduleMap))
        {
            pc->AddInput(d);
            auto it = precompileds.find(d);
            if (it != precompileds.end()) 
                pc->AddInput(it->second);
        }
        f->component.commands.push_back(pc);
    }
    for(auto &p : componentsToCompile)
    {
        auto &component = *p;
        if(component.type != "library" && GetParameter("build-" + component.type) == "false") {
            continue;
        }
        auto includes = getIncludePathsFor(component);
        std::vector<File *> objects;
        if (component.type == "unittest" || component.type == "fuzzer") {
            // Unit tests get access to includes from a component's private folder
            includes.insert((component.root / "../src").string());
        }
        for(auto &f : component.files)
        {
            if(!File::isTranslationUnit(f->path))
                continue;
            fs::path outputFile = "build/" + GetParameter("name") + "/obj/" + getObjNameFor(*f);
            File *of = project.CreateFile(component, outputFile);
            std::shared_ptr<PendingCommand> pc = std::make_shared<PendingCommand>(hash, getCompileCommand(GetCompilerFor(f->path.extension().string()), outputFile.generic_string(), f, includes, !f->moduleName.empty() || !f->imports.empty() || !f->modImports.empty()));
            objects.push_back(of);
            pc->AddOutput(of);
            pc->AddInput(f);
            for(auto &d : GetDependencies(f, moduleMap))
            {
                pc->AddInput(d);
                auto it = precompileds.find(d);
                if (it != precompileds.end()) 
                    pc->AddInput(it->second);
            }
            component.commands.push_back(pc);
        }
        if(!objects.empty())
        {
            std::string command;
            fs::path outputFile;
            std::shared_ptr<PendingCommand> pc;
            if(component.type == "library")
            {
                outputFile = "build/" + GetParameter("name") + "/lib/" + getLibNameFor(component);
                command = getArchiverCommand(GetParameter("archiver"), outputFile.generic_string(), objects);
                pc = std::make_shared<PendingCommand>(hash, command);
            }
            else
            {
                outputFile = "build/" + GetParameter("name") + "/bin/" + getExeNameFor(component);
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
                command = getLinkerCommand(GetParameter("linker"), outputFile.generic_string(), objects, linkDeps);
                pc = std::make_shared<PendingCommand>(hash, command);

                for(auto &d : linkDeps)
                {
                    for(auto &c : d)
                    {
                        if(c != &component && !c->isHeaderOnly() && !c->isPredefComponent)
                        {
                            pc->AddInput(project.CreateFile(*c, "build/" + GetParameter("name") + "/lib/" + getLibNameFor(*c)));
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
            component.commands.push_back(pc);
            if(component.type == "unittest" && GetParameter("run-unittest") == "true")
            {
                command = outputFile.string();
                pc = std::make_shared<PendingCommand>(hash, getUnittestCommand(command));
                outputFile += ".log";
                pc->AddInput(libraryFile);
                component.commands.push_back(pc);
            }
        }
    }
}
