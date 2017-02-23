#include "Analysis.h"
#include "Component.h"
#include <vector>

void MapFilesToComponents(std::unordered_map<std::string, Component *> &components, std::unordered_map<std::string, File>& files) {
    for (auto &fp : files) {
        std::string nameCopy = fp.first;
        size_t slashPos = nameCopy.find_last_of('/');
        while (slashPos != nameCopy.npos) {
            nameCopy.resize(slashPos);
            auto it = components.find(nameCopy);
            if (it != components.end()) {
                fp.second.component = it->second;
                it->second->files.insert(&fp.second);
                break;
            }
            slashPos = nameCopy.find_last_of('/');
        }
    }
}

void MapIncludesToDependencies(std::unordered_map<std::string, std::string> &includeLookup,
                               std::map<std::string, std::vector<std::string>> &ambiguous,
                               std::unordered_map<std::string, Component *> &components, 
                               std::unordered_map<std::string, File>& files) {
    for (auto &fp : files) {
        for (auto &p : fp.second.rawIncludes) {
            // If this is a non-pointy bracket include, see if there's a local match first. 
            // If so, it always takes precedence, never needs an include path added, and never is ambiguous (at least, for the compiler).
            std::string fullFilePath = (boost::filesystem::path(fp.first).parent_path() / p.first).generic_string();
            if (!p.second && files.count(fullFilePath)) {
                // This file exists as a local include.
                File* dep = &files.find(fullFilePath)->second;
                dep->hasInclude = true;
                fp.second.dependencies.insert(dep);
            } else {
                // We need to use an include path to find this. So let's see where we end up.
                std::string lowercaseInclude;
                std::transform(p.first.begin(), p.first.end(), std::back_inserter(lowercaseInclude), ::tolower);
                const std::string &fullPath = includeLookup[lowercaseInclude];
                if (fullPath == "INVALID") {
                    // We end up in more than one place. That's an ambiguous include then.
                    ambiguous[lowercaseInclude].push_back(fp.first);
                } else if (fullPath.find("GENERATED:") == 0) {
                    // We end up in a virtual file - it's not actually there yet, but it'll be generated.
                    if (fp.second.component) {
                        fp.second.component->buildAfters.insert(fullPath.substr(10));
                        Component *c = components["./" + fullPath.substr(10)];
                        if (c) {
                            fp.second.component->privDeps.insert(c);
                        }
                    }
                } else if (files.count(fullPath)) {
                    File *dep = &files.find(fullPath)->second;
                    fp.second.dependencies.insert(dep);

                    std::string inclpath = fullPath.substr(0, fullPath.size() - p.first.size() - 1);
                    if (inclpath.size() == dep->component->root.generic_string().size()) {
                        inclpath = ".";
                    } else if (inclpath.size() > dep->component->root.generic_string().size() + 1) {
                        inclpath = inclpath.substr(dep->component->root.generic_string().size() + 1);
                    } else {
                        inclpath = "";
                    }
                    if (!inclpath.empty()) {
                        dep->includePaths.insert(inclpath);
                    }

                    if (fp.second.component != dep->component) {
                        fp.second.component->privDeps.insert(dep->component);
                        dep->component->privLinks.insert(fp.second.component);
                        dep->hasExternalInclude = true;
                    }
                    dep->hasInclude = true;
                }
                // else we don't know about it. Probably a system include of some sort.
            }
        }
    }
}

void PropagateExternalIncludes(std::unordered_map<std::string, File>& files) {
    bool foundChange;
    do {
        foundChange = false;
        for (auto &fp : files) {
            if (fp.second.hasExternalInclude && fp.second.component) {
                for (auto &dep : fp.second.dependencies) {
                    if (!dep->hasExternalInclude && dep->component == fp.second.component) {
                        dep->hasExternalInclude = true;
                        foundChange = true;
                    }
                }
            }
        }
    } while (foundChange);
}


#include "Component.h"

void CreateIncludeLookupTable(std::unordered_map<std::string, File>& files,
                              std::unordered_map<std::string, std::string> &includeLookup,
                              std::unordered_map<std::string, std::set<std::string>> &collisions) {
    for (auto &p : files) {
        std::string lowercasePath;
        std::transform(p.first.begin(), p.first.end(), std::back_inserter(lowercasePath), ::tolower);
        const char *pa = lowercasePath.c_str();
        while ((pa = strstr(pa + 1, "/"))) {
            std::string &ref = includeLookup[pa + 1];
            if (ref.size() == 0) {
                ref = p.first;
            } else {
                collisions[pa + 1].insert(p.first);
                if (ref != "INVALID") {
                    collisions[pa + 1].insert(ref);
                }
                ref = "INVALID";
            }
        }
    }
}


