#pragma once

#include <functional>
#include <mutex>
#include <string>
#include <vector>

struct PendingCommand;
class Reporter;
class Process;

class Executor
{
public:
    Executor(size_t jobcount, Reporter& reporter, std::function<void()> OnComplete);
    ~Executor();
    void Run(PendingCommand *cmd);
    void Start();
    bool Busy();

private:
    void RunMoreCommands();
    std::mutex m;
    std::vector<PendingCommand *> commands;
    std::vector<Process *> activeProcesses;
    Reporter& reporter;
    std::function<void()> OnComplete;
};

