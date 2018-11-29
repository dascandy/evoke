#pragma once

#include "Component.h"
#include "File.h"
#include "PendingCommand.h"
#include "utils.h"

#include <fw/filesystem.hpp>
#include <ostream>
#include <string>
#include <unordered_map>

class Project
{
public:
    Project(const std::string &root);
    ~Project();
    void Reload();
    File *CreateFile(Component &c, filesystem::path p);
    filesystem::path projectRoot;
    std::unordered_map<std::string, Component> components;
    std::unordered_set<std::string> unknownHeaders;
    std::unordered_map<std::string, File> files;
    std::vector<PendingCommand *> buildPipeline;
    std::unordered_map<std::string, std::vector<std::string>> ambiguous;

    bool IsCompilationUnit(const std::string &ext);
    bool IsCode(const std::string &ext);
    bool IsSystemComponent(const std::string &name) const;
    void dumpJsonCompileDb(std::ostream &os);
    void dumpCMakeListsTxt(const GlobalOptions &opts);

private:
    void LoadFileList();
    bool CreateModuleMap(std::unordered_map<std::string, File *> &moduleMap);
    void MapImportsToModules(std::unordered_map<std::string, File *> &moduleMap);
    void CreateIncludeLookupTable(std::unordered_map<std::string, std::string> &includeLookup,
                                  std::unordered_map<std::string, std::set<std::string>> &collisions);
    void MapIncludesToDependencies(std::unordered_map<std::string, std::string> &includeLookup,
                                   std::unordered_map<std::string, std::vector<std::string>> &ambiguous);
    void PropagateExternalIncludes();
    void ExtractPublicDependencies();
    void ExtractIncludePaths();
    void ReadCodeFrom(File &f, const char *buffer, size_t buffersize);
    void ReadCode(std::unordered_map<std::string, File> &files, const filesystem::path &path, Component &comp);
    bool IsItemBlacklisted(const filesystem::path &path);
    friend std::ostream &operator<<(std::ostream &os, const Project &p);
};

std::ostream &operator<<(std::ostream &os, const Project &p);
