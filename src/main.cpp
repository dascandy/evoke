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
        state = State::Reloading;
        components.clear();
        files.clear();
        LoadFileList(components, files, projectRoot);

        std::unordered_map<std::string, std::string> includeLookup;
        std::unordered_map<std::string, std::set<std::string>> collisions;
        CreateIncludeLookupTable(files, includeLookup, collisions);
        ForgetEmptyComponents(components);
        std::unordered_map<std::string, std::vector<std::string>> ambiguous;
        MapIncludesToDependencies(includeLookup, ambiguous, files);
        if (!ambiguous.empty()) {
          state = State::AmbiguousHeaders;
          // TODO: write ambiguous log to log file
        } else {
          PropagateExternalIncludes(files);
          ExtractPublicDependencies(components);
          state = State::Good;
        }
    }
    boost::filesystem::path projectRoot;
    std::unordered_map<std::string, Component> components;
    std::unordered_map<std::string, File> files;
    enum class State {
      Reloading,
      Good,
      AmbiguousHeaders,
    } state = State::Reloading;
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


