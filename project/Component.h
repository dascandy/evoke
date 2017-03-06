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

struct File;

struct Component {
    explicit Component(const boost::filesystem::path &path);
    std::string GetName() const;
    boost::filesystem::path root;
    std::unordered_set<Component *> pubDeps;
    std::unordered_set<Component *> privDeps;
    std::unordered_set<File *> files;
    std::string type;
};

Component &AddComponentDefinition(std::unordered_map<std::string, Component> &components,
                                  const boost::filesystem::path &path );

std::ostream& operator<<(std::ostream& os, const Component& component);


