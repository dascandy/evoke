#pragma once

#include <string>
#include <vector>
#include <functional>

class Finder {
  public:
    Finder(const std::vector<std::string>& builds, std::function<void(const std::string&, const std::string&)> OnServer) ;
};



