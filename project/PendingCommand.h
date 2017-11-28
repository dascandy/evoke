#pragma once

#include <vector>
#include <string>
class File;

struct PendingCommand {
public:
  PendingCommand(const std::string& command);
  void AddInput(File* input);
  void AddOutput(File* output);
private:
  void SignalRecheck();
  std::vector<File*> inputs;
  std::vector<File*> outputs;
  public:
  std::string commandToRun;
  private:
  enum State {
    Unknown,
    ToBeRun,
    Running,
    Done
  } state = Unknown;
  void TriggerRebuild();
};

