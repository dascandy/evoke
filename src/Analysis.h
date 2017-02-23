#pragma once

#include <unordered_map>
#include <string>
#include <vector>
class Component;
class File;

void MapFilesToComponents(std::unordered_map<std::string, Component *> &components, std::unordered_map<std::string, File>& files);

void MapIncludesToDependencies(std::unordered_map<std::string, std::string> &includeLookup,
                               std::unordered_map<std::string, std::vector<std::string>> &ambiguous,
                               std::unordered_map<std::string, Component *> &components, 
                               std::unordered_map<std::string, File>& files);

void PropagateExternalIncludes(std::unordered_map<std::string, File>& files);


