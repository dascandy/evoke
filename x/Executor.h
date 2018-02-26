#pragma once

struct Process {
public:
  Process(const std::string& cmd, const std::string& statefile, std::function<void()> onComplete);
  ~Process();
private:
  State state = Running;
  int pid;
  std::vector<char> outbuffer;
};

class Executor {
public:
  Process& Start(const std::string& command);
};


