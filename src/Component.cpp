#include "Component.h"

Component::Component(const boost::filesystem::path &path)
        : root(path), type("library") {}

Component &AddComponentDefinition(std::unordered_map<std::string, Component> &components,
                                  const boost::filesystem::path &path) {
    return components.emplace(path.generic_string(), path.generic_string()).first->second;
}

void ExtractPublicDependencies(std::unordered_map<std::string, Component> &components) {
    for (auto &c : components) {
        bool hasExtIncludes = false;
        Component &comp = c.second;
        for (auto &fp : comp.files) {
            if (fp->hasExternalInclude) {
                hasExtIncludes = true;
                for (auto &dep : fp->dependencies) {
                    comp.privDeps.erase(&dep->component);
                    comp.pubDeps.insert(&dep->component);
                }
            }
        }
        comp.pubDeps.erase(&comp);
        comp.privDeps.erase(&comp);
        comp.type = hasExtIncludes ? "library" : "executable";
    }
}

std::ostream& operator<<(std::ostream& os, const Component& component) {
    os << "Component " << component.GetName() << " built as " << component.type;
    if (!component.pubDeps.empty()) {
      os << "\n  Exposes:";
      for (auto& d : component.pubDeps) {
          os << " " << d->GetName();
      }
    }
    if (!component.privDeps.empty()) {
      os << "\n  Uses:";
      for (auto& d : component.privDeps) {
          os << " " << d->GetName();
      }
    }
    bool anyExternal = false, anyHeader = false, anySource = false;
    for (auto& d : component.files) {
        if (d->hasExternalInclude) anyExternal = true;
        else if (d->hasInclude) anyHeader = true;
        else anySource = true;
    }
    if (anyExternal) {
        os << "\n  Exposed headers:";
        for (auto& d : component.files) {
            if (d->hasExternalInclude) 
                os << " " << d->path.generic_string();
        }
    }
    if (anyHeader) {
        os << "\n  Private headers:";
        for (auto& d : component.files) {
            if (!d->hasExternalInclude && d->hasInclude) 
                os << " " << d->path.generic_string();
        }
    }
    if (anySource) {
        os << "\n  Sources:";
        for (auto& d : component.files) {
            if (!d->hasInclude) 
                os << " " << d->path.generic_string();
        }
    }
    os << "\n\n";
    return os;
}


