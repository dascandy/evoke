#include "Project.h"

#include <fw/filesystem.hpp>
#include <ostream>
#include <unordered_map>

class Toolset;

struct CMakeProjectExporter
{
    explicit CMakeProjectExporter(const Project &project);
    std::string LookupLibraryName(const std::string &componentName);
    void createCMakeListsFiles(const Toolset &toolset);
private:
    void dumpDoNotModifyWarning(std::ostream &os);
    void extractSystemComponents(const Component &comp, std::unordered_set<std::string> &visited, std::vector<const Component *> &results) const;
    std::vector<const Component *> extractSystemComponents() const;
    void dumpTargetIncludes(std::ostream &os, const std::string &target, const std::string &access, const std::unordered_set<std::string> &includes);
    void dumpTargetLibraries(std::ostream &os, const std::string &target, const std::string &access, const std::unordered_set<Component *> components);
    void extractHierarchicalNames();
    bool IsSystemComponent(const std::string &name) const;
    std::unordered_map<std::string, std::string> hierarchical_names;
    static constexpr const char *cmakeSystemProjectPrefix = "evoke_system_lib_";
    static constexpr const char *cmakelists_txt = "CMakeLists.txt";
    const Project &project_;
};

