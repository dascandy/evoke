#pragma once

#include "Component.h"

#include <fw/filesystem.hpp>
#include <string>
#include <unordered_map>
#include <unordered_set>

struct Component;

struct File
{
private:
    File(const filesystem::path &path, Component &component) :
        path(path),
        component(component),
        hasExternalInclude(false),
        hasInclude(false)
    {
    }
    friend class Project;
    void AddImportStmt(bool withPointyBrackets, const std::string &filename)
    {
        rawImports.insert(std::make_pair(filename, withPointyBrackets));
    }
    void AddIncludeStmt(bool withPointyBrackets, const std::string &filename)
    {
        rawIncludes.insert(std::make_pair(filename, withPointyBrackets));
    }
    void SetModule(const std::string &moduleName, bool exported)
    {
        this->moduleName = moduleName;
        moduleExported = exported;
    }
    void AddImport(const std::string &importName, bool exported)
    {
        if(importName[0] == ':')
        {
            imports[moduleName + importName] = exported;
        }
        else
        {
            imports[importName] = exported;
        }
    }

public:
    static bool isTranslationUnit(const filesystem::path &);
    static bool isHeader(const filesystem::path &);
    bool isTranslationUnit() const;
    bool isHeader() const;
    std::time_t lastwrite()
    {
        if(lastwrite_ == 0)
        {
            filesystem::error_code ec;
#if defined(_MSC_VER)
            lastwrite_ = filesystem::last_write_time(path, ec);
#else
            auto ftime = filesystem::last_write_time(path, ec);
            lastwrite_ = decltype(ftime)::clock::to_time_t(ftime);
#endif
        }
        return lastwrite_;
    }
    // Cache for the last write time of this file.
    std::time_t lastwrite_ = 0;
    // Full path from the root of the project to this file. Always starts with "./".
    filesystem::path path;
    // Module name, if any.
    std::string moduleName;
    // Whether the module (given above) is marked for export.
    bool moduleExported = false;
    // Imports of modules by name (module name)
    std::unordered_map<std::string, bool> imports;
    // Includes of headers by filename
    std::unordered_map<std::string, bool> rawIncludes;
    // Import of headers by filename. Specifically C++ imports; Obj-C imports are treated as includes (as they do not need a precompile step)
    std::unordered_map<std::string, bool> rawImports;
    // Files where the compilation of this TU depends on the precompiled dependency (and not the dependency itself). Think modules.
    std::unordered_map<std::string, File *> modImports;
    // Files where the compilation of this TU depends on the actual dependency directly. Think includes.
    std::unordered_map<std::string, File *> dependencies;
    // All include paths needed to make the include and import statements to this file function.
    std::unordered_set<std::string> includePaths;
    // If this is a generated file, this is the generator making it.
    PendingCommand *generator = nullptr;
    // All commands that must be flagged for recheck when this file is updated. (think downstream dependencies)
    std::vector<PendingCommand *> listeners;
    // Reference to the component this file belongs to.
    Component &component;
    // Indication that this file needs to be made publically available as part of the component interface
    bool hasExternalInclude = false;
    // Indication that this file is used in an include statement somewhere. I think this is obsolete knowledge? It should not be used at least.
    bool hasInclude = false;
    // State of the file mentioned. NotFound/Source are for files that have no generator, ToRebuild/Rebuilding/Error/Done are the states for a file with generator.
    enum State
    {
        Unknown,
        NotFound,
        Source,
        ToRebuild,
        Rebuilding,
        Error,
        Done,
    } state = Source;
    void SignalRebuild(State newState)
    {
        state = newState;
    }
};
