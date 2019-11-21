#pragma once

#include "fw/filesystem.hpp"

#include <string>

std::string as_dotted(std::string);

fs::path removeDot(fs::path const &p);

