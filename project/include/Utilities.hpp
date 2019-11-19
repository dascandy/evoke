#pragma once

#include <fw/filesystem.hpp>
#include <string>

std::string GetNameFromPath(const fs::path &path, char separator = '_');