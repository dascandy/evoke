#pragma once

#include <string>
#include <fw/filesystem.hpp>

std::string GetNameFromPath(const filesystem::path &path, char separator = '_');