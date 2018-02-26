#pragma once

#include <vector>
#include <string>
#include <ostream>
class File;

struct PendingCommand {
public:
  PendingCommand(const std::string& command);
  void AddInput(File* input);
  void AddOutput(File* output);
private:
  std::vector<File*> inputs;
  std::vector<File*> outputs;
  void Check();
public:
  std::string commandToRun;
  enum State {
    Unknown,
    ToBeRun,
    Running,
    Done
  } state = Unknown;
};

std::ostream& operator<<(std::ostream& os, const PendingCommand&);

