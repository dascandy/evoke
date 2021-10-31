#pragma once

#include <future>
#include <fw/filesystem.hpp>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

struct PendingCommand;
class Reporter;
class Process;

class Executor
{
public:
    Executor(size_t jobcount, uint64_t memoryLimit, Reporter &reporter);
    ~Executor();
    void Run(std::shared_ptr<PendingCommand> cmd);
    std::future<void> Mode(bool isDaemon);

    bool AllSuccess();
    void RunMoreCommands();
    std::mutex m;
    std::vector<std::shared_ptr<PendingCommand>> commands;
private:
    std::vector<std::unique_ptr<Process>> activeProcesses;
    Reporter &reporter;
    bool daemonMode = false;
    bool commandsSorted = false;
    size_t memoryLimit;
    size_t memoryFree, memoryTotal;
};
