#pragma once

#include <vector>
#include <string>
class File;

struct PendingCommand {
  std::vector<File*> inputs;
  std::vector<File*> outputs;
  std::string commandToRun;
  enum State {
    Unknown,
    ToBeRun,
    Running,
    Done
  } state = Unknown;
};

