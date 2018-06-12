#pragma once

#include <string>
#include <vector>

struct Configuration {
private:
  Configuration();
  void LoadDefaults();
public:
  static Configuration& Get();
  std::vector<std::string> blacklist;
  std::string toolchain;
  std::string compileFlags;
};


