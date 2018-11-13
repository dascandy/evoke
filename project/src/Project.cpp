#include "Project.h"

#include "Component.h"
#include "Configuration.h"
#include "File.h"
#include "known.h"

#include <algorithm>
#include <fw/filesystem.hpp>
#include <iostream>
#include <map>
#ifndef _WIN32
#    include <fcntl.h>
#    include <sys/mman.h>
#    include <unistd.h>
#endif

Project::Project(const std::string &rootpath)
{
    projectRoot = rootpath;
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

    std::unordered_map<std::string, std::string> includeLookup;
    std::unordered_map<std::string, std::set<std::string>> collisions;
    CreateIncludeLookupTable(includeLookup, collisions);
    MapIncludesToDependencies(includeLookup, ambiguous);
    if(!ambiguous.empty())
    {
        fprintf(stderr, "Ambiguous includes found!\n");
        for(auto &i : ambiguous)
        {
            fprintf(stderr, "Include name %s could point to %zu files -", i.first.c_str(), i.second.size());
            for(auto &s : i.second)
            {
                fprintf(stderr, " %s", s.c_str());
            }
            fprintf(stderr, "\n");
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

File *Project::CreateFile(Component &c, boost::filesystem::path p)
{
    std::string subpath = p.string();
    if(subpath[0] == '.' && subpath[1] == '/')
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

void Project::ReadCode(std::unordered_map<std::string, File> &files, const boost::filesystem::path &path, Component &comp)
{
#ifdef _WIN32
    File &f = files.insert(std::make_pair(path.generic_string(), File(path))).first->second;
    std::string buffer;
    buffer.resize(filesystem::file_size(path));
    {
        filesystem::ifstream(path).read(&buffer[0], buffer.size());
    }
    ReadCodeFrom(f, buffer.data(), buffer.size(), withLoc);
#else
    File &f = files.emplace(path.generic_string().substr(2), File(path.generic_string().substr(2), comp)).first->second;
    comp.files.insert(&f);
    int fd = open(path.c_str(), O_RDONLY);
    size_t fileSize = filesystem::file_size(path);
    void *p = mmap(NULL, fileSize, PROT_READ, MAP_PRIVATE, fd, 0);
    ReadCodeFrom(f, static_cast<const char *>(p), fileSize);
    munmap(p, fileSize);
    close(fd);
#endif
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

bool Project::IsCode(const std::string &ext)
{
    static const std::unordered_set<std::string> exts = {".c", ".C", ".cc", ".cpp", ".cppm", ".m", ".mm", ".h", ".H", ".hpp", ".hh", ".tcc", ".ipp", ".inc"};
    return exts.count(ext) > 0;
}

bool Project::IsCompilationUnit(const std::string &ext)
{
    static const std::unordered_set<std::string> exts = {".c", ".C", ".cc", ".cpp", ".cppm", ".m", ".mm"};
    return exts.count(ext) > 0;
}

static Component *GetComponentFor(std::unordered_map<std::string, Component> &components, boost::filesystem::path path)
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
    for(boost::filesystem::recursive_directory_iterator it("."), end;
        it != end;
        ++it)
    {
        boost::filesystem::path parent = it->path().parent_path();
        // skip hidden files and dirs
        std::string fileName = it->path().filename().generic_string();
        if((fileName.size() >= 2 && fileName[0] == '.') || IsItemBlacklisted(it->path()))
        {
            it.disable_recursion_pending();
            continue;
        }

        if(boost::filesystem::is_directory(it->status()))
        {
            if(boost::filesystem::is_directory(it->path() / "include") || boost::filesystem::is_directory(it->path() / "src"))
            {
                components.emplace(it->path().c_str(), it->path());
                if(boost::filesystem::is_directory(it->path() / "test"))
                {
                    components.emplace((it->path() / "test").c_str(), it->path() / "test").first->second.type = "unittest";
                }
            }
        }
        else if(boost::filesystem::is_regular_file(it->status()) && IsCode(it->path().extension().generic_string().c_str()))
        {
            Component *component = GetComponentFor(components, it->path());
            if(component)
            {
                ReadCode(files, it->path(), *component);
            }
            else
            {
                fprintf(stderr, "Found file %s outside of any component\n", it->path().c_str());
            }
        }
    }
}

bool Project::CreateModuleMap(std::unordered_map<std::string, File *> &moduleMap)
{
    bool error = false;
    for(auto &f : files)
    {
        if(!f.second.moduleName.empty())
        {
            auto &entry = moduleMap[f.second.moduleName];
            if(entry)
            {
                fprintf(stderr, "Found second definition for module %s - found first in %s\n", f.second.path.c_str(), entry->path.c_str());
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
                f.second.modImports.insert(target);
            }
            else
            {
                fprintf(stderr, "Could not find module %s imported by %s\n", import.first.c_str(), f.second.path.c_str());
            }
        }
    }
}

static std::map<std::string, Component *> PredefComponentList()
{
    std::map<std::string, Component *> list;
    Component *boost_system = new Component("boost_system", true);
    Component *boost_filesystem = new Component("boost_filesystem", true);
    boost_filesystem->pubDeps.insert(boost_system);
    list["boost/filesystem.hpp"] = boost_filesystem;
    list["boost/process.hpp"] = new Component("boost_process");

    Component *crypto = new Component("crypto", true);
    list["md5.h"] = crypto;

    Component *ssl = new Component("ssl", true);
    ssl->pubDeps.insert(crypto);
    list["openssl/conf.h"] = ssl;
    list["openssl/ssl.h"] = ssl;
    list["openssl/engine.h"] = ssl;
    list["openssl/dh.h"] = ssl;
    list["openssl/err.h"] = ssl;
    list["openssl/rsa.h"] = ssl;
    list["openssl/x509v3.h"] = ssl;
    list["openssl/x509_vfy.h"] = ssl;

    list["zlib.h"] = new Component("z", true);

    list["mysql/mysql.h"] = new Component("mysqlclient", true);
    list["sdl2/sdl.h"] = new Component("SDL2", true);
    list["sdl2/sdl_opengl.h"] = new Component("GL", true);
    list["gl/glew.h"] = new Component("GLEW", true);
    return list;
}

static Component *GetPredefComponent(const boost::filesystem::path &path)
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
        for(auto &p : fp.second.rawImports)
        {
            // If this is a non-pointy bracket include, see if there's a local match first.
            // If so, it always takes precedence, never needs an include path added, and never is ambiguous (at least, for the compiler).
            std::string fullFilePath = (boost::filesystem::path(fp.first).parent_path() / p.first).generic_string();
            if(!p.second && files.count(fullFilePath))
            {
                // This file exists as a local include.
                File *dep = &files.find(fullFilePath)->second;
                dep->hasInclude = true;
                fp.second.modImports.insert(dep);
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
                    fp.second.modImports.insert(dep);

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
            std::string fullFilePath = (boost::filesystem::path(fp.first).parent_path() / p.first).generic_string();
            if(!p.second && files.count(fullFilePath))
            {
                // This file exists as a local include.
                File *dep = &files.find(fullFilePath)->second;
                dep->hasInclude = true;
                fp.second.dependencies.insert(dep);
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
                    fp.second.dependencies.insert(dep);

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
                    if(!dep->hasExternalInclude && &dep->component == &fp.second.component)
                    {
                        dep->hasExternalInclude = true;
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
                    comp.privDeps.erase(&dep->component);
                    comp.pubDeps.insert(&dep->component);
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
