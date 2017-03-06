#pragma once

#include <boost/filesystem/path.hpp>
#include <unordered_map>
#include <string>
#include <ostream>
#include "Component.h"
#include "File.h"

struct Project {
    Project();
    void Reload();
    boost::filesystem::path projectRoot;
    std::unordered_map<std::string, Component> components;
    std::unordered_map<std::string, File> files;
};

std::ostream& operator<<(std::ostream& os, const Project& p);


