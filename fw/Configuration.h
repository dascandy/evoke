#pragma once

#include <string>
#include <vector>

struct Configuration {
  Configuration();
  static Configuration& Get();
  std::vector<std::string> blacklist;
};


