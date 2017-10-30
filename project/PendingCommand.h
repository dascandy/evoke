#pragma once

#include <vector>
#include <string>
class File;

struct PendingCommand {
public:
  PendingCommand(const std::string& command);
  void AddInput(File* input);
  void AddOutput(File* output);
  void SignalRecheck();
private:
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

