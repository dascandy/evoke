#include "Project.h"

#include "Component.h"
#include "Configuration.h"
#include "File.h"
#include "PredefComponents.h"
#include "known.h"

#include <algorithm>
#include <fstream>
#include <fw/filesystem.hpp>
#include <iostream>
#include <map>

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

Project::Project(const std::string &rootpath)
{
    projectRoot = rootpath;
    filesystem::current_path(projectRoot);
    Reload();
}

Project::~Project()
{
}

void Project::Reload()
{
    unknownHeaders.clear();
    components.clear();
    files.clear();
    ambiguous.clear();
    LoadFileList();

    std::unordered_map<std::string, File *> moduleMap;
    CreateModuleMap(moduleMap);
    MapImportsToModules(moduleMap);
    MoveIncludeToImport();

    std::unordered_map<std::string, std::string> includeLookup;
    std::unordered_map<std::string, std::set<std::string>> collisions;
    CreateIncludeLookupTable(includeLookup, collisions);
    MapIncludesToDependencies(includeLookup, ambiguous);
    if(!ambiguous.empty())
    {
        std::cerr << "Ambiguous includes found!\n";

        for(auto &i : ambiguous)
        {
            std::cerr << "Include name " << i.first.c_str() << " could point to " << i.second.size() << " files:";

            for(auto &s : i.second)
            {
                std::cerr << s.c_str();
            }

            std::cerr << '\n';
        }
    }
    PropagateExternalIncludes();
    ExtractPublicDependencies();
    ExtractIncludePaths();

    for(auto it = components.begin(); it != components.end();)
    {
        if(it->second.files.empty())
        {
            it = components.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

File *Project::CreateFile(Component &c, filesystem::path p)
{
    std::string subpath = p.string();
    if(subpath[0] == '.' && (subpath[1] == '/' || subpath[1] == '\\'))
        subpath = subpath.substr(2);
    File f(p, c);
    auto f2 = files.emplace(p.string(), std::move(f));
    return &f2.first->second;
}

std::ostream &operator<<(std::ostream &os, const Project &p)
{
    for(auto &c : p.components)
    {
        os << c.second << "\n";
    }
    os << "Pipeline:\n";
    for(auto &c : p.buildPipeline)
    {
        os << c << "\n";
    }
    return os;
}

void Project::ReadCode(std::unordered_map<std::string, File> &files, const filesystem::path &path, Component &comp)
{
    File &f = files.emplace(path.generic_string().substr(2), File(path.generic_string().substr(2), comp)).first->second;
    comp.files.insert(&f);

    size_t fileSize = filesystem::file_size(path.string());
    if (fileSize == 0) return; // Boost::interprocess fails (and throws) on mapping a 0-byte file

    using namespace boost::interprocess;
    file_mapping file(path.string().c_str(), read_only);
    mapped_region region(file, read_only);

    ReadCodeFrom(f, static_cast<const char *>(region.get_address()), region.get_size());
}

bool Project::IsItemBlacklisted(const filesystem::path &path)
{
    std::string pathS = path.generic_string();
    std::string fileName = path.filename().generic_string();
    for(auto &s : Configuration::Get().blacklist)
    {
        if(pathS.compare(2, s.size(), s) == 0)
        {
            return true;
        }
        if(s == fileName)
            return true;
    }
    return false;
}

bool Project::IsSystemComponent(const Component &comp) const
{
    const auto name = comp.GetHierarchicalName();
    auto result = (components.find(name) == components.cend());
    if(result)
    {
        auto altName = "./" + name;
        result = (components.find(altName) == components.cend());
    }
    return result;
}

static Component *GetComponentFor(std::unordered_map<std::string, Component> &components, filesystem::path path)
{
    Component *rv = nullptr;
    size_t matchLength = 0;
    for(auto &p : components)
    {
        if(p.first.size() > matchLength && p.first.size() < path.string().size() && path.string().compare(0, p.first.size(), p.first) == 0)
        {
            rv = &p.second;
            matchLength = p.first.size();
        }
    }
    return rv;
}

void Project::LoadFileList()
{
    std::string root = ".";
    for(filesystem::recursive_directory_iterator it("."), end;
        it != end;
        ++it)
    {
        filesystem::path parent = it->path().parent_path();
        // skip hidden files and dirs
        std::string fileName = it->path().filename().generic_string();
        if((fileName.size() >= 2 && fileName[0] == '.') || IsItemBlacklisted(it->path()))
        {
            it.disable_recursion_pending();
            continue;
        }

        if(filesystem::is_directory(it->status()))
        {
            if(filesystem::is_directory(it->path() / "include") || filesystem::is_directory(it->path() / "src"))
            {
                components.emplace(it->path().string(), it->path());
                if(filesystem::is_directory(it->path() / "test"))
                {
                    components.emplace((it->path() / "test").string(), it->path() / "test").first->second.type = "unittest";
                }
            }
        }
        else if(filesystem::is_regular_file(it->status()) && (File::isHeader(it->path()) || File::isTranslationUnit(it->path())))
        {
            Component *component = GetComponentFor(components, it->path());
            if(component)
            {
                ReadCode(files, it->path(), *component);
            }
            else
            {
                std::cerr << "Found file " << it->path().c_str() << " outside of any component\n";
            }
        }
    }
}

bool Project::CreateModuleMap(std::unordered_map<std::string, File *> &moduleMap)
{
    bool error = false;
    for(auto &f : files)
    {
        if(!f.second.moduleName.empty() && f.second.moduleExported)
        {
            auto &entry = moduleMap[f.second.moduleName];
            if(entry)
            {
                std::cerr << "Found second definition for module " << f.second.path.c_str() << " found first in " << entry->path.c_str() << '\n';
                error = true;
            }
            else
            {
                entry = &f.second;
            }
        }
    }
    return error;
}

void Project::MapImportsToModules(std::unordered_map<std::string, File *> &moduleMap)
{
    for(auto &f : files)
    {
        for(auto &import : f.second.imports)
        {
            File *target = moduleMap[import.first];
            if(target)
            {
                f.second.modImports.insert(std::make_pair(import.first, target));
            }
            else
            {
                std::cerr << "Could not find module " << import.first.c_str() << " imported by " << f.second.path.c_str() << '\n';
            }
        }
    }
}

void Project::MoveIncludeToImport()
{
    std::unordered_set<File *> importedHeaders;
    // Find all headers imported anywhere anyhow as an import (ie, precompiled)
    for(auto &f : files)
        for(auto &d : f.second.modImports)
            importedHeaders.insert(d.second);

    // Move those that map to an imported file to the imports section, so it will view it as an import
    for(auto &f : files)
    {
        for(auto it = f.second.dependencies.begin(); it != f.second.dependencies.end();)
        {
            if(importedHeaders.find(it->second) != importedHeaders.end())
            {
                f.second.modImports.insert(*it);
                f.second.dependencies.erase(it);
            }
            else
                ++it;
        }
    }
}

static Component *GetPredefComponent(const filesystem::path &path)
{
    static auto list = PredefComponentList();
    if(list.find(path.string()) != list.end())
        return list.find(path.string())->second;
    return nullptr;
}

void Project::MapIncludesToDependencies(std::unordered_map<std::string, std::string> &includeLookup,
                                        std::unordered_map<std::string, std::vector<std::string>> &ambiguous)
{
    for(auto &fp : files)
    {
        for (auto& p : fp.second.modImports) {
            if(&fp.second.component != &p.second->component)
            {
                fp.second.component.privDeps.insert(&p.second->component);
                p.second->hasExternalInclude = true;
            }
            p.second->hasInclude = true;
        }
        // TODO: add the by-name imports here too as a component level dependency
        for(auto &p : fp.second.rawImports)
        {
            // If this is a non-pointy bracket include, see if there's a local match first.
            // If so, it always takes precedence, never needs an include path added, and never is ambiguous (at least, for the compiler).
            std::string fullFilePath = (filesystem::path(fp.first).parent_path() / p.first).generic_string();
            if(!p.second && files.count(fullFilePath))
            {
                // This file exists as a local include.
                File *dep = &files.find(fullFilePath)->second;
                dep->hasInclude = true;
                fp.second.modImports.insert(std::make_pair(p.first, dep));
            }
            else
            {
                // We need to use an include path to find this. So let's see where we end up.
                std::string lowercaseInclude;
                std::transform(p.first.begin(), p.first.end(), std::back_inserter(lowercaseInclude), ::tolower);
                const std::string &fullPath = includeLookup[lowercaseInclude];
                if(fullPath == "INVALID")
                {
                    // We end up in more than one place. That's an ambiguous include then.
                    ambiguous[lowercaseInclude].push_back(fp.first);
                }
                else if(GetPredefComponent(lowercaseInclude))
                {
                    Component *comp = GetPredefComponent(lowercaseInclude);
                    fp.second.component.privDeps.insert(comp);
                    // TODO: we need to precompile this too now? No hook for it yet...
                }
                else if(files.count(fullPath))
                {
                    File *dep = &files.find(fullPath)->second;
                    fp.second.modImports.insert(std::make_pair(p.first, dep));

                    std::string inclpath = fullPath.substr(0, fullPath.size() - p.first.size() - 1);
                    if(inclpath.size() == dep->component.root.generic_string().size())
                    {
                        inclpath = ".";
                    }
                    else if(inclpath.size() > dep->component.root.generic_string().size() + 1)
                    {
                        inclpath = inclpath.substr(dep->component.root.generic_string().size() + 1);
                    }
                    else
                    {
                        inclpath = "";
                    }
                    if(!inclpath.empty())
                    {
                        dep->includePaths.insert(inclpath);
                    }

                    if(&fp.second.component != &dep->component)
                    {
                        fp.second.component.privDeps.insert(&dep->component);
                        dep->hasExternalInclude = true;
                    }
                    dep->hasInclude = true;
                }
                else if(!IsKnownHeader(p.first))
                {
                    unknownHeaders.insert(p.first);
                }
            }
        }
        for(auto &p : fp.second.rawIncludes)
        {
            // If this is a non-pointy bracket include, see if there's a local match first.
            // If so, it always takes precedence, never needs an include path added, and never is ambiguous (at least, for the compiler).
            std::string fullFilePath = (filesystem::path(fp.first).parent_path() / p.first).generic_string();
            if(!p.second && files.count(fullFilePath))
            {
                // This file exists as a local include.
                File *dep = &files.find(fullFilePath)->second;
                dep->hasInclude = true;
                fp.second.dependencies.insert(std::make_pair(p.first, dep));
            }
            else
            {
                // We need to use an include path to find this. So let's see where we end up.
                std::string lowercaseInclude;
                std::transform(p.first.begin(), p.first.end(), std::back_inserter(lowercaseInclude), ::tolower);
                const std::string &fullPath = includeLookup[lowercaseInclude];
                if(fullPath == "INVALID")
                {
                    // We end up in more than one place. That's an ambiguous include then.
                    ambiguous[lowercaseInclude].push_back(fp.first);
                }
                else if(GetPredefComponent(lowercaseInclude))
                {
                    Component *comp = GetPredefComponent(lowercaseInclude);
                    fp.second.component.privDeps.insert(comp);
                }
                else if(files.count(fullPath))
                {
                    File *dep = &files.find(fullPath)->second;
                    fp.second.dependencies.insert(std::make_pair(p.first, dep));

                    std::string inclpath = fullPath.substr(0, fullPath.size() - p.first.size() - 1);
                    if(inclpath.size() == dep->component.root.generic_string().size())
                    {
                        inclpath = ".";
                    }
                    else if(inclpath.size() > dep->component.root.generic_string().size() + 1)
                    {
                        inclpath = inclpath.substr(dep->component.root.generic_string().size() + 1);
                    }
                    else
                    {
                        inclpath = "";
                    }
                    if(!inclpath.empty())
                    {
                        dep->includePaths.insert(inclpath);
                    }

                    if(&fp.second.component != &dep->component)
                    {
                        fp.second.component.privDeps.insert(&dep->component);
                        dep->hasExternalInclude = true;
                    }
                    dep->hasInclude = true;
                }
                else if(!IsKnownHeader(p.first))
                {
                    unknownHeaders.insert(p.first);
                }
            }
        }
    }
}

void Project::PropagateExternalIncludes()
{
    bool foundChange;
    do
    {
        foundChange = false;
        for(auto &fp : files)
        {
            if(fp.second.hasExternalInclude)
            {
                for(auto &dep : fp.second.dependencies)
                {
                    if(!dep.second->hasExternalInclude && &dep.second->component == &fp.second.component)
                    {
                        dep.second->hasExternalInclude = true;
                        foundChange = true;
                    }
                }
            }
        }
    } while(foundChange);
}

void Project::CreateIncludeLookupTable(std::unordered_map<std::string, std::string> &includeLookup,
                                       std::unordered_map<std::string, std::set<std::string>> &collisions)
{
    for(auto &p : files)
    {
        std::string lowercasePath;
        std::transform(p.first.begin(), p.first.end(), std::back_inserter(lowercasePath), ::tolower);
        const char *pa = lowercasePath.c_str();
        while((pa = strstr(pa + 1, "/")))
        {
            std::string &ref = includeLookup[pa + 1];
            if(ref.size() == 0)
            {
                ref = p.first;
            }
            else
            {
                collisions[pa + 1].insert(p.first);
                if(ref != "INVALID")
                {
                    collisions[pa + 1].insert(ref);
                }
                ref = "INVALID";
            }
        }
    }
}

void Project::ExtractPublicDependencies()
{
    for(auto &c : components)
    {
        bool hasExtIncludes = false;
        Component &comp = c.second;
        for(auto &fp : comp.files)
        {
            if(fp->hasExternalInclude)
            {
                hasExtIncludes = true;
                for(auto &dep : fp->dependencies)
                {
                    comp.privDeps.erase(&dep.second->component);
                    comp.pubDeps.insert(&dep.second->component);
                }
            }
        }
        comp.pubDeps.erase(&comp);
        comp.privDeps.erase(&comp);
        comp.type = comp.root.filename().string() == "test" ? "unittest" : (hasExtIncludes || (*comp.root.begin() == "packages")) ? "library" : "executable";
    }
}

void Project::ExtractIncludePaths()
{
    for(auto &c : components)
    {
        Component &comp = c.second;
        for(auto &fp : comp.files)
        {
            if(fp->hasInclude)
            {
                (fp->hasExternalInclude ? comp.pubIncl : comp.privIncl).insert(fp->includePaths.begin(), fp->includePaths.end());
            }
        }
        for(auto &s : comp.pubIncl)
        {
            comp.privIncl.erase(s);
        }
    }
}
