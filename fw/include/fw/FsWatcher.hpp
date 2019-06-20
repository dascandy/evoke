#pragma once

#include "fw/filesystem.hpp"
#include <functional>

enum class Change {
  Changed,
  Created,
  Deleted
};

void FsWatch(filesystem::path path, std::function<void(filesystem::path, Change)> onEvent);

