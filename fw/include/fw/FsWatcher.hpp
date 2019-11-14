#pragma once

#include "fw/filesystem.hpp"
#include <functional>

enum class Change {
  Changed,
  Created,
  Deleted
};

void FsWatch(fs::path path, std::function<void(fs::path, Change)> onEvent);

