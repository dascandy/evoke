#include "Component.h"

Component::Component(const boost::filesystem::path &path)
        : root(path), name(""), type("library") {}

Component &AddComponentDefinition(std::unordered_map<std::string, Component> &components,
                                  const boost::filesystem::path &path) {
    return components.emplace(path.generic_string(), path.generic_string()).first->second;
}

void ExtractPublicDependencies(std::unordered_map<std::string, Component *> &components) {
    for (auto &c : components) {
        Component *comp = c.second;
        for (auto &fp : comp->files) {
            if (fp->hasExternalInclude) {
                for (auto &dep : fp->dependencies) {
                    comp->privDeps.erase(&dep->component);
                    comp->pubDeps.insert(&dep->component);
                }
            }
        }
        comp->pubDeps.erase(comp);
        comp->privDeps.erase(comp);
    }
}

