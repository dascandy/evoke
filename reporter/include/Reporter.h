#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <vector>
#include <unordered_set>

struct PendingCommand;
class Reporter
{
public:
    virtual void SetConcurrencyCount(size_t count) = 0;
    virtual void SetRunningCommand(size_t channel, std::shared_ptr<PendingCommand> command) = 0;
    virtual void ReportCommand(size_t channel, std::shared_ptr<PendingCommand> command) = 0;
    virtual void ReportCommandQueue(std::vector<std::shared_ptr<PendingCommand>>& ) {}
    virtual void ReportUnknownHeaders(const std::unordered_set<std::string>&) {}
    virtual ~Reporter() = default;
    static std::unique_ptr<Reporter> Get(const std::string &name);

protected:
    Reporter() = default;
};

class ConsoleReporter : public Reporter
{
public:
    ConsoleReporter();
    ~ConsoleReporter();
    void SetConcurrencyCount(size_t count) override;
    void SetRunningCommand(size_t channel, std::shared_ptr<PendingCommand> command) override;
    void ReportCommand(size_t channel, std::shared_ptr<PendingCommand> command) override;
    void ReportUnknownHeaders(const std::unordered_set<std::string>&) override;

private:
    void Redraw();
    std::vector<std::shared_ptr<PendingCommand>> activeProcesses;
};

class GitteryReporter : public Reporter
{
public:
    GitteryReporter();
    ~GitteryReporter();
    void SetConcurrencyCount(size_t ) override {}
    void SetRunningCommand(size_t , std::shared_ptr<PendingCommand> ) override {}
    void ReportCommand(size_t channel, std::shared_ptr<PendingCommand> command) override;
};

class SimpleReporter : public Reporter
{
public:
    void SetConcurrencyCount(size_t count) override;
    void SetRunningCommand(size_t channel, std::shared_ptr<PendingCommand> command) override;
    void ReportCommand(size_t channel, std::shared_ptr<PendingCommand> command) override;
};

class DaemonConsoleReporter : public Reporter
{
public:
    DaemonConsoleReporter();
    void SetConcurrencyCount(size_t count) override;
    void SetRunningCommand(size_t channel, std::shared_ptr<PendingCommand> command) override;
    void ReportCommand(size_t channel, std::shared_ptr<PendingCommand> command) override;
    void ReportCommandQueue(std::vector<std::shared_ptr<PendingCommand>>& allCommands) override;
    void ReportUnknownHeaders(const std::unordered_set<std::string>&) override;

private:
    void Redraw();
    std::vector<std::shared_ptr<PendingCommand>> activeProcesses;
};

class IDEReporter : public Reporter
{
public:
    void SetConcurrencyCount(size_t count) override;
    void SetRunningCommand(size_t channel, std::shared_ptr<PendingCommand> command) override;
    void ReportCommand(size_t channel, std::shared_ptr<PendingCommand> command) override;
};

