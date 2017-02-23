#pragma once

#include <boost/filesystem.hpp>
#include <algorithm>
#include <iostream>
#include <map>
#include <regex>
#include <set>
#include <stdio.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// Forward reference:
struct Component;

struct File {
    File(const boost::filesystem::path& path)
    : path(path)
    , component(NULL)
    , loc(0)
    , includeCount(0)
    , hasExternalInclude(false)
    , hasInclude(false)
    {
    }

    void AddIncludeStmt(bool withPointyBrackets, const std::string& filename) {
        rawIncludes.insert(std::make_pair(filename, withPointyBrackets));
    }
    boost::filesystem::path path;
    std::map<std::string, bool> rawIncludes;
    std::unordered_set<File *> dependencies;
    std::unordered_set<std::string> includePaths;
    Component *component;
    size_t loc;
    size_t includeCount;
    bool hasExternalInclude;
    bool hasInclude;
};

struct Component {
    std::string NiceName(char sub) const;

    std::string QuotedName() const;

    std::string CmakeName() const;

    explicit Component(const boost::filesystem::path &path);

    boost::filesystem::path root;
    std::string name;
    // deps are the dependencies of your component
    std::unordered_set<Component *> pubDeps;
    std::unordered_set<Component *> privDeps;
    // links are the components which are using your component
    std::unordered_set<Component *> pubLinks;
    std::unordered_set<Component *> privLinks;
    std::unordered_set<Component *> circulars;
    std::set<std::string> buildAfters;
    std::unordered_set<File *> files;
    size_t loc() const {
        size_t l = 0;
        for (auto f : files) { l += f->loc; }
        return l;
    }
    bool recreate;
    bool hasAddonCmake;
    std::string type;
    size_t index, lowlink;
    bool onStack;
};

std::vector<std::string> SortedNiceNames(const std::unordered_set<Component *> &comps);

Component &AddComponentDefinition(std::unordered_map<std::string, Component *> &components,
                                  const boost::filesystem::path &path );

size_t NodesWithCycles(std::unordered_map<std::string, Component *> &components);

void ExtractPublicDependencies(std::unordered_map<std::string, Component *> &components);

void CreateIncludeLookupTable(std::unordered_map<std::string, File>& files,
                              std::unordered_map<std::string, std::string> &includeLookup,
                              std::unordered_map<std::string, std::set<std::string>> &collisions);



