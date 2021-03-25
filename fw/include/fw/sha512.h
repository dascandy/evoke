#pragma once

#include "filesystem.hpp"
#include <array>
#include <cstdint>

std::array<uint8_t, 64> sha512(const fs::path& path);


