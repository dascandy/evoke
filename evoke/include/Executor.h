#pragma once

#include <functional>
#include <future>
#include <fw/filesystem.hpp>
#include <mutex>
#include <string>
#include <vector>

struct PendingCommand;
class Reporter;
class Process;

class Executor
{
public:
    Executor(size_t jobcount, Reporter &reporter);
    ~Executor();
    void Run(PendingCommand *cmd);
    std::future<void> Start();

private:
    void RunMoreCommands();
    std::mutex m;
    std::vector<PendingCommand *> commands;
    std::vector<Process *> activeProcesses;
    Reporter &reporter;
    std::promise<void> done;
};
