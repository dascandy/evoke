#pragma once

#include <algorithm>
#include <fw/filesystem.hpp>
#include <iostream>
#include <regex>
#include <set>
#include <stdio.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct File;
struct PendingCommand;

struct Component
{
public:
    Component(const filesystem::path &path, bool isBinary = false);
    std::string GetName() const;
    std::string GetCMakeSubdirectoryName() const;
    bool isHeaderOnly() const;
    filesystem::path root;
    std::unordered_set<File *> files;
    std::vector<std::shared_ptr<PendingCommand>> commands;
    std::unordered_set<Component *> pubDeps, privDeps;
    std::unordered_set<std::string> pubIncl, privIncl;
    std::string type;
    bool buildSuccess;
    bool isBinary;
    std::string accumulatedErrors;
    std::string name;
    std::string cmake_subdir_name;
};

std::ostream &operator<<(std::ostream &os, const Component &component);

std::vector<std::vector<Component *>> GetTransitiveAllDeps(Component &c);
std::vector<std::vector<Component *>> GetTransitivePubDeps(Component &c);
std::set<std::string> getIncludePathsFor(Component &component);
