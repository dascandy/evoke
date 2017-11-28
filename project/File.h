#pragma once

#include <boost/filesystem.hpp>
#include <string>
#include <unordered_map>
#include <unordered_set>

struct Component;

struct File {
private:
  File(const boost::filesystem::path& path, Component& component)
  : path(path)
  , component(component)
  , hasExternalInclude(false)
  , hasInclude(false)
  {
  }
  friend class Project;
  void AddIncludeStmt(bool withPointyBrackets, const std::string& filename) {
      rawIncludes.insert(std::make_pair(filename, withPointyBrackets));
  }
public:
  boost::filesystem::path path;
  std::unordered_map<std::string, bool> rawIncludes;
  std::unordered_set<File *> dependencies;
  std::unordered_set<std::string> includePaths;
  PendingCommand* generator = nullptr;
  std::vector<PendingCommand*> listeners;
  Component &component;
  bool hasExternalInclude;
  bool hasInclude;
  enum State {
    Source,
    Unknown,
    ToRebuild,
    Rebuilding,
    Error,
    Done,
  } state = Source;
  void SignalRebuild(State newState);
};


