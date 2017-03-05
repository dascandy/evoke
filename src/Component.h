#pragma once

#include <boost/filesystem.hpp>
#include <algorithm>
#include <iostream>
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
    File(const boost::filesystem::path& path, Component& component)
    : path(path)
    , component(component)
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
    Component &component;
    bool hasExternalInclude;
    bool hasInclude;
};

struct Component {
    explicit Component(const boost::filesystem::path &path);
    std::string GetName() const {
        if (root.size() < 3) 
            return boost::filesystem::absolute(root).filename().string();
        return root.generic_string().substr(2);
    }
    boost::filesystem::path root;
    std::unordered_set<Component *> pubDeps;
    std::unordered_set<Component *> privDeps;
    std::unordered_set<File *> files;
    std::string type;
};

Component &AddComponentDefinition(std::unordered_map<std::string, Component> &components,
                                  const boost::filesystem::path &path );

void ExtractPublicDependencies(std::unordered_map<std::string, Component> &components);

void CreateIncludeLookupTable(std::unordered_map<std::string, File>& files,
                              std::unordered_map<std::string, std::string> &includeLookup,
                              std::unordered_map<std::string, std::set<std::string>> &collisions);

std::ostream& operator<<(std::ostream& os, const Component& component);


