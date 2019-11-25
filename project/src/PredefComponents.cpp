#include "PredefComponents.h"
#include "fw/FileParser.h"
#include "Component.h"

#include <map>
#include <string>

std::map<std::string, Component *> PredefComponentList()
{
    std::map<std::string, Component *> list;
    static std::map<std::string, Component> components;
    Component* current = nullptr;
    ParseFile("packages.conf", [&current](const std::string& tag){
        components.insert(std::make_pair(tag, Component(tag)));
        current = &components.find(tag)->second;
    }, [&current, &list](const std::string& key, const std::string& value) {
        if (key == "files") {
            for (auto& f : splitWithQuotes(value)) {
                list[f] = current;
            }
        } else if (key == "paths") {
            for (auto& path : splitWithQuotes(value)) {
                current->pubIncl.insert(path);
            }
        } else if (key == "binary") {
            current->isBinary = true;
        } else if (key == "public_dependencies") {
            for (auto& f : splitWithQuotes(value)) {
                if (components.count(f) == 0) {
                    components.insert(std::make_pair(f, Component(f)));
                }
                current->pubDeps.insert(&components.find(f)->second);
            }
        }
    });

    return list;
}
