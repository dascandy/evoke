#pragma once

#include <functional>
#include <mutex>
#include <string>
#include <vector>

struct PendingCommand;

class Process;

class Executor
{
public:
    Executor();
    ~Executor();
    void Run(PendingCommand *cmd);
    void Start();
    bool Busy();

private:
    void RunMoreCommands();
    std::mutex m;
    std::vector<PendingCommand *> commands;
    std::vector<Process *> activeProcesses;
};
