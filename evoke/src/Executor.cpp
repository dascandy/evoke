#include "Executor.h"

#include "PendingCommand.h"
#include "Reporter.h"

#include <boost/process.hpp>
#include <cstring>
#include <functional>
#include <thread>

#ifdef __linux__
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/sysinfo.h>

#endif

static std::promise<void> done;

class Process
{
public:
    Process(std::shared_ptr<PendingCommand> pc, const std::string &filename, const std::string &cmd, std::function<void(Process *)> onComplete);
private:
    void run();
public:
    std::shared_ptr<PendingCommand> pc;
    PendingCommand::FileRecord record;
    std::function<void(Process *)> onComplete;
    std::string filename;
    boost::process::ipstream pipe_stream;
    boost::process::child child;
    int errorcode = 0;
    std::string outbuffer;
    enum State
    {
        Running,
        Done
    };
    State state = Running;
};

Process::Process(std::shared_ptr<PendingCommand> pc, const std::string &filename, const std::string &cmd, std::function<void(Process *)> onComplete) 
: pc(pc)
, record(*pc->result)
, onComplete(onComplete)
, filename(filename)
, child(cmd, (boost::process::std_out & boost::process::std_err) > pipe_stream)
{
    record.toolsetHash = pc->toolsetHash;
    record.tuHash = pc->inputs[0]->hash;
    record.output.clear();
    std::thread([this] { run(); }).detach();
}

void Process::run()
{
    std::string line;
    while(std::getline(pipe_stream, line))
        record.output += line + "\n";

    try {
#ifdef __linux__
        struct rusage r = {};
        [[maybe_unused]] int rv = wait4(child.id(), &record.errorcode, 0, &r);
        
        record.timeEstimate += (r.ru_utime.tv_sec + r.ru_stime.tv_sec) + (r.ru_utime.tv_usec + r.ru_stime.tv_usec) * 0.000001;
        record.spaceNeeded += r.ru_maxrss * 1024;
        record.measurementCount++;
#else
        child.wait();
        record.errorcode = child.exit_code();
#endif
    } catch (const std::exception& e) {
        record.output += e.what();
        record.errorcode = -1;
    }
    state = Done;
    // The callback will cause this object to be destructed, so move out the callback before invoking it
    auto x = std::move(onComplete);
    x(this);
}

Executor::Executor(size_t jobcount, uint64_t memoryLimit, Reporter &reporter) 
: reporter(reporter)
, memoryLimit(memoryLimit)
{
#ifdef __linux__
    struct sysinfo info;
    sysinfo(&info);
    memoryFree = (info.freeram + info.bufferram + info.freeswap) * info.mem_unit;
    memoryTotal = (info.totalram + info.totalswap) * info.mem_unit;

    if (memoryLimit == 0) {
        this->memoryLimit = memoryFree;
    }
#endif
    activeProcesses.resize(jobcount);
    reporter.SetConcurrencyCount(jobcount);
}

Executor::~Executor()
{
}

bool Executor::AllSuccess() {
    for (auto& command : commands) {
        if (command->result->errorcode) return false;
    }
    return true;
}

void Executor::Run(std::shared_ptr<PendingCommand> cmd)
{
    if (cmd->result->spaceNeeded > memoryFree) {
        fprintf(stderr, "PROBLEM: Memory (including swap) not enough to build %s.\nRequired: %zu MB, Available: %zu MB, Total: %zu MB\n", cmd->outputs[0]->path.c_str(), cmd->result->spaceNeeded / 1000000, memoryFree / 1000000, memoryTotal / 1000000);
    }
    commandsSorted = false;
    commands.push_back(cmd);
}

std::future<void> Executor::Mode(bool isDaemon)
{
    daemonMode = isDaemon;
    if (isDaemon) {
#ifndef _WIN32
    // TODO: Add the Windows side of signal handling
        void (*signalHandler)(int) = [](int signo){ done.set_value(); std::cout << " SIGNAL " << signo << "\n";};
        signal(SIGTERM, signalHandler);
        signal(SIGHUP, signalHandler);
        signal(SIGINT, signalHandler);
        signal(SIGQUIT, signalHandler);
        signal(SIGTSTP, signalHandler);
#endif
    }
    return done.get_future();
}

void Executor::NewGeneration() {
//    commands.clear();
// HOW?!
    std::terminate();
}

void Executor::RunMoreCommands()
{
    if (not commandsSorted) {
        std::sort(commands.begin(), commands.end(), [](std::shared_ptr<PendingCommand>& a, std::shared_ptr<PendingCommand>& b) {
          return a->timeToComplete() > b->timeToComplete();
        });
        commandsSorted = true;
    }
    SaveCommandResultDb();
    reporter.ReportCommandQueue(commands);
    size_t n = 0;
    uint64_t memoryLeft = memoryLimit;
    bool somethingRunning = false;
    for (auto& p : activeProcesses) {
        if (p) {
            somethingRunning = true;
            memoryLeft -= p->pc->result->spaceNeeded;
        }
    }
    for(auto &c : commands)
    {
        while(n != activeProcesses.size() && activeProcesses[n])
            ++n;
        if(n == activeProcesses.size())
            break;
        // TODO: take into account its relative load
        if(c->ReadyToStart() &&  // There are no prerequisites that are obviously missing or broken, and it's not already running
           (c->result->spaceNeeded < memoryLeft || // We have space left to run it
            !somethingRunning)) // or there is not enough space to run it but it's the only thing to run, so we might as well try
        {
            somethingRunning = true;
            memoryLeft -= c->result->spaceNeeded;
            c->state = PendingCommand::Running;
            for(auto &o : c->outputs)
            {
                fs::create_directories(o->path.parent_path());
            }
            reporter.SetRunningCommand(n, c);
            activeProcesses[n] = std::make_unique<Process>(c, c->outputs[0]->path.filename().string(), c->commandToRun, [this, n, c](Process *t) {
                std::lock_guard<std::mutex> l(m);
                auto self = std::move(activeProcesses[n]);
                c->SetResult(std::move(t->record));
                reporter.ReportCommand(n, c);
                reporter.SetRunningCommand(n, nullptr);
                RunMoreCommands();
            });
        }
        else
        {
            ;
        }
    }

    if (not somethingRunning and not daemonMode) 
        done.set_value();
}
