#include "Toolset.h"

std::unique_ptr<Toolset> GetToolsetByName(const std::string& name) {
  if (name == "android") {
    return std::make_unique<AndroidToolset>();
  } else if (name == "windows") {
    return std::make_unique<WindowsToolset>();
  } else {
    return std::make_unique<UbuntuToolset>();
  }
}


