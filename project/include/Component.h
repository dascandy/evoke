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
    Component(const fs::path &path, bool isBinary = false);
    std::string GetName() const;
    bool isHeaderOnly() const;
    fs::path root;
    std::unordered_set<File *> files;
    std::vector<std::shared_ptr<PendingCommand>> commands;
    std::unordered_set<Component *> pubDeps, privDeps;
    std::unordered_set<std::string> pubIncl;
    std::string type;
    bool buildSuccess;
    bool isBinary;
    bool isPredefComponent;
    std::string accumulatedErrors;
    std::string name;
};

std::ostream &operator<<(std::ostream &os, const Component &component);

std::vector<std::vector<Component *>> GetTransitiveAllDeps(Component &c);
std::vector<std::vector<Component *>> GetTransitivePubDeps(Component &c);
std::set<std::string> getIncludePathsFor(Component &component);
