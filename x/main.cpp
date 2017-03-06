#include "Analysis.h"
#include "Component.h"
#include "Configuration.h"
#include <boost/filesystem.hpp>
#include "Input.h"
#include <iostream>

struct Project {
    Project(int, const char**)
    {
        projectRoot = boost::filesystem::current_path();
        Reload();
    }
    void Reload() {
        components.clear();
        files.clear();
        LoadFileList(components, files, projectRoot);

        std::unordered_map<std::string, std::string> includeLookup;
        std::unordered_map<std::string, std::set<std::string>> collisions;
        CreateIncludeLookupTable(files, includeLookup, collisions);
        ForgetEmptyComponents(components);
        std::unordered_map<std::string, std::vector<std::string>> ambiguous;
        MapIncludesToDependencies(includeLookup, ambiguous, files);
        for (auto &i : ambiguous) {
            for (auto &c : collisions[i.first]) {
                files.find(c)->second.hasInclude = true; // There is at least one include that might end up here.
            }
        }
        PropagateExternalIncludes(files);
        ExtractPublicDependencies(components);
    }
    void UnloadProject() {
    }
    boost::filesystem::path projectRoot;
    std::unordered_map<std::string, Component> components;
    std::unordered_map<std::string, File> files;
};

std::ostream& operator<<(std::ostream& os, const Project& p) {
    for (auto& c : p.components) {
        os << c.second << "\n";
    }
    return os;
}

int main(int argc, const char **argv) {
    Project op(argc, argv);
    std::cout << op;
    return 0;
}
