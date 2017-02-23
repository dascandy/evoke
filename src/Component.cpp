#include "Component.h"

std::string Component::NiceName(char sub) const {
    if (root.string() == ".") {
        return std::string("ROOT");
    }

    std::string nicename = root.generic_string().c_str() + 2;
    std::replace(nicename.begin(), nicename.end(), '/', sub);
    return nicename;
}

std::string Component::QuotedName() const {
    return std::string("\"") + NiceName('.') + std::string("\"");
}

std::string Component::CmakeName() const {
    return (recreate || name.empty()) ? NiceName('.') : name;
}

Component::Component(const boost::filesystem::path &path)
        : root(path), name(""), recreate(false), hasAddonCmake(false), type("library"), index(0), lowlink(0), onStack(false) {
}

std::vector<std::string> SortedNiceNames(const std::unordered_set<Component *> &comps) {
    std::vector<std::string> ret;
    ret.resize(comps.size());
    std::transform(comps.begin(), comps.end(), ret.begin(), [](const Component *comp) -> std::string {
        return comp->NiceName('.');
    });
    std::sort(ret.begin(), ret.end());

    return ret;
}

Component &AddComponentDefinition(std::unordered_map<std::string, Component *> &components,
                                  const boost::filesystem::path &path) {
    Component *&comp = components[path.generic_string()];
    if (!comp) {
        comp = new Component(path);
    }
    return *comp;
}

size_t NodesWithCycles(std::unordered_map<std::string, Component *> &components) {
    size_t count = 0;
    for (auto &c : components) {
        if (!c.second->circulars.empty()) {
            count++;
        }
    }
    return count;
}

void ExtractPublicDependencies(std::unordered_map<std::string, Component *> &components) {
    for (auto &c : components) {
        Component *comp = c.second;
        for (auto &fp : comp->files) {
            if (fp->hasExternalInclude) {
                for (auto &dep : fp->dependencies) {
                    comp->privDeps.erase(dep->component);
                    comp->pubDeps.insert(dep->component);
                }
            }
        }
        comp->pubDeps.erase(comp);
        comp->privDeps.erase(comp);
    }
}

