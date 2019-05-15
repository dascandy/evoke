#pragma once

#include "fw/filesystem.hpp"

#include <string>

std::string as_dotted(std::string);

filesystem::path removeDot(filesystem::path const &p);

