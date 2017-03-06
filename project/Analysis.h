#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <set>

class Component;
class File;

void MapIncludesToDependencies(std::unordered_map<std::string, std::string> &includeLookup,
                               std::unordered_map<std::string, std::vector<std::string>> &ambiguous,
                               std::unordered_map<std::string, File>& files);

void PropagateExternalIncludes(std::unordered_map<std::string, File>& files);

void ExtractPublicDependencies(std::unordered_map<std::string, Component> &components);

void CreateIncludeLookupTable(std::unordered_map<std::string, File>& files,
                              std::unordered_map<std::string, std::string> &includeLookup,
                              std::unordered_map<std::string, std::set<std::string>> &collisions);


