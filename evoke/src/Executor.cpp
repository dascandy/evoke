#include "Executor.h"

#include "PendingCommand.h"

#include <cstring>
#include <functional>
#include <thread>
#include <boost/process.hpp>

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
    while (pipe_stream && std::getline(pipe_stream, line) && !line.empty())
        outbuffer += line;

    child.wait();
    errorcode = child.exit_code();
    state = Done;
    // The callback will delete this object, so move out the callback before invoking it
    auto x = std::move(onComplete);
    x(this);
}

Executor::Executor()
{
    activeProcesses.resize(std::max(4u, std::thread::hardware_concurrency()));
}

Executor::~Executor()
{
}

void Executor::Run(PendingCommand *cmd)
{
    std::lock_guard<std::mutex> l(m);
    commands.push_back(cmd);
}

bool Executor::Busy()
{
    // What if they just all finished and just haven't started another yet?
    std::lock_guard<std::mutex> l(m);
    for(auto &c : activeProcesses)
    {
        if(c && c->state != Process::Done)
            return true;
    }
    return false;
}

void Executor::Start()
{
    RunMoreCommands();
}

void Executor::RunMoreCommands()
{
    std::lock_guard<std::mutex> l(m);
    auto it = activeProcesses.begin();
    for(auto &c : commands)
    {
        while(it != activeProcesses.end() && *it)
            ++it;
        if(it == activeProcesses.end())
            break;
        // TODO: take into account its relative load
        if(c->CanRun())
        {
            c->state = PendingCommand::Running;
            for(auto &o : c->outputs)
            {
                boost::filesystem::create_directories(o->path.parent_path());
            }
            *it = new Process(c->outputs[0]->path.filename().string(), c->commandToRun, [this, it, c](Process *t) {
                *it = nullptr;
                // TODO: print errors from this command first
                if(t->errorcode || !t->outbuffer.empty())
                {
                    t->outbuffer.push_back(0);
                    printf("\n\nError while running command for %s:\n$ %s\n%s\n", c->outputs[0]->path.filename().string().c_str(), c->commandToRun.c_str(), t->outbuffer.data());
                }
                c->SetResult(t->errorcode == 0);
                delete t;
                RunMoreCommands();
            });
        }
        else
        {
            ;
        }
    }

    size_t screenWidth = 80;
    size_t w = screenWidth / activeProcesses.size();
    size_t active = 0;
    for(auto &t : activeProcesses)
        if(t)
            active++;

    printf("%zu concurrent tasks, %zu active, %zu commands left to run\n", activeProcesses.size(), active, commands.size());
    if (w == 0) {
        // If you have more cores than horizontal characters, we can't display one task per character. Instead make the horizontal line a "usage" bar representing proportional task use.
        size_t activeCount = 0;
        for (auto& t : activeProcesses) {
            if (t) activeCount++;
        }
        std::cout << std::string((screenWidth-1) * activeCount / activeProcesses.size(), '*') << std::string(screenWidth - (screenWidth-1) * activeCount / activeProcesses.size() - 1, ' ');
    } else {
        for(auto &t : activeProcesses)
        {
            if (w >= 10) 
            {
                std::string file;
                if(t)
                {
                    file = ((Process *)t)->filename;
                    if(file.size() > w - 2)
                        file.resize(w - 2);
                }
                while(file.size() < w - 2)
                    file.push_back(' ');
                printf("\033[1;37m[\033[0m%s\033[1;37m]\033[0m", file.c_str());
            } else if (w >= 3) {
                printf("\033[1;37m[\033[0m%s\033[1;37m]\033[0m", t ? "*" : "_");
            } else if (w >= 1) {
                printf("\033[1;37m%s\033[0m", t ? "*" : "_");
            }
        }
    }
    printf("\r\033[1A");
    fflush(stdout);
}
