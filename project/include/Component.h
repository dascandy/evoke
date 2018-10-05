#pragma once

#include <boost/filesystem.hpp>
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
  Component(const boost::filesystem::path &path, bool isBinary = false);
  ~Component();
  std::string GetName() const;
  bool isHeaderOnly() const;
  boost::filesystem::path root;
  std::unordered_set<File *> files;
  std::vector<PendingCommand*> commands;
  std::unordered_set<Component *> pubDeps, privDeps;
  std::unordered_set<std::string> pubIncl, privIncl;
  std::string type;
  bool buildSuccess;
  bool isBinary;
  std::string accumulatedErrors;
};

std::ostream& operator<<(std::ostream& os, const Component& component);

std::vector<std::vector<Component*>> GetTransitiveAllDeps(Component& c);
std::vector<std::vector<Component*>> GetTransitivePubDeps(Component& c);
std::set<std::string> getIncludePathsFor(Component& component);


