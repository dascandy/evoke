#pragma once

#include <boost/filesystem.hpp>
#include <string>
#include <unordered_map>

struct File;
struct Component;

void ForgetEmptyComponents(std::unordered_map<std::string, Component> &components);
void LoadFileList(std::unordered_map<std::string, Component> &components,
                  std::unordered_map<std::string, File>& files,
                  const boost::filesystem::path& sourceDir);
                  


