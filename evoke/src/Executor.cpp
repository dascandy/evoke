#include "Executor.h"

#include "PendingCommand.h"
#include "Reporter.h"

#include <boost/process.hpp>
#include <cstring>
#include <functional>
#include <thread>

class Process
{
public:
    Process(const std::string &filename, const std::string &cmd, std::function<void(Process *)> onComplete);

private:
    void run();

public:
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

Process::Process(const std::string &filename, const std::string &cmd, std::function<void(Process *)> onComplete) :
    onComplete(onComplete),
    filename(filename),
    child(cmd, boost::process::std_out > pipe_stream)
{
    std::thread([this] { run(); }).detach();
}

void Process::run()
{
    std::string line;
    while(pipe_stream && std::getline(pipe_stream, line) && !line.empty())
        outbuffer += line;

    child.wait();
    errorcode = child.exit_code();
    state = Done;
    // The callback will cause this object to be destructed, so move out the callback before invoking it
    auto x = std::move(onComplete);
    x(this);
}

Executor::Executor(size_t jobcount, Reporter &reporter) :
    reporter(reporter)
{
    activeProcesses.resize(jobcount);
    reporter.SetConcurrencyCount(jobcount);
}

Executor::~Executor()
{
}

void Executor::Run(std::shared_ptr<PendingCommand> cmd)
{
    std::lock_guard<std::mutex> l(m);
    commands.push_back(cmd);
}

std::future<void> Executor::Start()
{
    std::lock_guard<std::mutex> l(m);
    RunMoreCommands();
    return done.get_future();
}

void Executor::RunMoreCommands()
{
    size_t n = 0;
    for(auto &c : commands)
    {
        while(n != activeProcesses.size() && activeProcesses[n])
            ++n;
        if(n == activeProcesses.size())
            break;
        // TODO: take into account its relative load
        if(c->CanRun())
        {
            c->state = PendingCommand::Running;
            for(auto &o : c->outputs)
            {
                filesystem::create_directories(o->path.parent_path());
            }
            reporter.SetRunningCommand(n, c);
            activeProcesses[n] = std::make_unique<Process>(c->outputs[0]->path.filename().string(), c->commandToRun, [this, n, c](Process *t) {
                std::lock_guard<std::mutex> l(m);
                c->SetResult(t->errorcode == 0);
                if(t->errorcode || !t->outbuffer.empty())
                {
                    t->outbuffer.push_back(0);
                    reporter.ReportFailure(c, t->errorcode, t->outbuffer.data());
                }
                reporter.SetRunningCommand(n, nullptr);
                activeProcesses[n].reset();
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

    done.set_value();
}
