#include "Project.h"
#include "Analysis.h"
#include "Component.h"
#include "File.h"
#include "Input.h"
#include <boost/filesystem.hpp>

Project::Project() {
    projectRoot = boost::filesystem::current_path();
    Reload();
}

void Project::Reload() {
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

std::ostream& operator<<(std::ostream& os, const Project& p) {
    for (auto& c : p.components) {
        os << c.second << "\n";
    }
    return os;
}


