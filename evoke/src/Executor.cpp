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
#endif

static std::promise<void> done;

class Process
{
public:
    Process(std::shared_ptr<PendingCommand> pc, const std::string &filename, const std::string &cmd, std::function<void(Process *, double, uint64_t)> onComplete);

private:
    void run();
public:
    std::shared_ptr<PendingCommand> pc;
    std::function<void(Process *, double, uint64_t)> onComplete;
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

Process::Process(std::shared_ptr<PendingCommand> pc, const std::string &filename, const std::string &cmd, std::function<void(Process *, double, uint64_t)> onComplete) :
    pc(pc),
    onComplete(onComplete),
    filename(filename),
    child(cmd, (boost::process::std_out & boost::process::std_err) > pipe_stream)
{
    std::thread([this] { run(); }).detach();
}

void Process::run()
{
    std::string line;
    while(std::getline(pipe_stream, line))
        outbuffer += line + "\n";

    double ctime = 0;
    unsigned long vsize = 0;
    try {
#ifdef __linux__
        struct rusage r;
        int rv = wait4(child.id(), &errorcode, 0, &r);

        ctime = (r.ru_utime.tv_sec + r.ru_stime.tv_sec) + (r.ru_utime.tv_usec + r.ru_stime.tv_usec) * 0.000001;
        vsize = r.ru_maxrss * 1024;
#else
        child.wait();
        errorcode = child.exit_code();
#endif
    } catch (const std::exception& e) {
        outbuffer = e.what();
        errorcode = -1;
    }
    state = Done;
    // The callback will cause this object to be destructed, so move out the callback before invoking it
    auto x = std::move(onComplete);
    x(this, ctime, vsize);
}

Executor::Executor(size_t jobcount, uint64_t memoryLimit, Reporter &reporter) 
: reporter(reporter)
, memoryLimit(memoryLimit)
{
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
    generation++;
    commands.clear();
    reporter.ReportCommandQueue(commands);
}

void Executor::RunMoreCommands()
{
    std::sort(commands.begin(), commands.end(), [](std::shared_ptr<PendingCommand>& a, std::shared_ptr<PendingCommand>& b) {
      return a->timeToComplete() < b->timeToComplete();
    });
    reporter.ReportCommandQueue(commands);
    size_t n = 0;
    uint64_t memoryLeft = memoryLimit;
    for (auto& p : activeProcesses) {
        if (p) memoryLeft -= p->pc->result->spaceNeeded;
    }
    for(auto &c : commands)
    {
        while(n != activeProcesses.size() && activeProcesses[n])
            ++n;
        if(n == activeProcesses.size())
            break;
        // TODO: take into account its relative load
        if(c->CanRun() && c->result->spaceNeeded < memoryLeft)
        {
            memoryLeft -= c->result->spaceNeeded;
            c->state = PendingCommand::Running;
            for(auto &o : c->outputs)
            {
                fs::create_directories(o->path.parent_path());
            }
            reporter.SetRunningCommand(n, c);
            activeProcesses[n] = std::make_unique<Process>(c, c->outputs[0]->path.filename().string(), c->commandToRun, [this, n, c, generationWhenStarted = generation](Process *t, double timeTaken, uint64_t spaceUsed) {
                std::lock_guard<std::mutex> l(m);
                auto self = std::move(activeProcesses[n]);
                if (generation == generationWhenStarted) {   // If the generation counter changed, then all our target pointers are stale. Don't talk to our command any more.
                  t->outbuffer.push_back(0);
                  c->SetResult(t->errorcode, t->outbuffer.data(), timeTaken, spaceUsed);
                  reporter.ReportCommand(n, c);
                } else {
                  reporter.ReportCommand(n, nullptr);
                }
                reporter.SetRunningCommand(n, nullptr);
                RunMoreCommands();
            });
        }
        else
        {
            ;
        }
    }

    for(auto &p : activeProcesses)
        if(p)
            return;

    if (!daemonMode)
        done.set_value();
}
