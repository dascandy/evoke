#pragma once

#include <filesystem>
#include <algorithm>
#include <iostream>
#include <regex>
#include <set>
#include <stdio.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct File;
struct PendingCommand;

struct Component {
public:
  Component(const std::filesystem::path &path);
  std::string GetName() const;
  std::filesystem::path root;
  std::unordered_set<Component *> pubDeps;
  std::unordered_set<Component *> privDeps;
  std::unordered_set<File *> files;
  std::vector<PendingCommand*> commands;
  std::string type;
};

std::ostream& operator<<(std::ostream& os, const Component& component);


