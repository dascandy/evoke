#include "Executor.h"

#include "PendingCommand.h"

#include <cstring>
#include <fcntl.h>
#include <functional>
#include <sys/types.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>

struct Process : public Task
{
public:
    Process(const std::string &filename, const std::string &cmd, const std::string &statefile, std::function<void(Task *)> onComplete) :
        onComplete(onComplete),
        filename(filename)
    {
        int outfd[2];
        pipe(outfd);
        if((pid = fork()) == 0)
        {
            close(0);
            dup2(outfd[1], 1);
            dup2(outfd[1], 2);
            close(outfd[0]);
            std::vector<char *> argv;
            size_t start = 0, end = cmd.find_first_of(" ");
            while(end != cmd.npos)
            {
                argv.push_back(strndup(cmd.data() + start, end - start));
                start = cmd.find_first_not_of(" ", end);
                end = cmd.find_first_of(" ", start);
            }
            argv.push_back(strdup(cmd.data() + start));
            argv.push_back(nullptr);
            execvp(argv[0], argv.data());
            abort();
        }
        close(outfd[1]);
        thread = std::thread([this, fd = outfd[0]] { run(fd); });
        thread.detach();
    }
    ~Process()
    {
        //    thread.join();
    }

private:
    void run(int fd)
    {
        char buffer[1024];
        while(true)
        {
            int bread = read(fd, buffer, sizeof(buffer));
            if(bread == 0)
                break;
            else if(bread < 0)
            {
                if(errno == EINTR)
                    continue;
            }
            outbuffer.insert(outbuffer.end(), buffer, buffer + bread);
        }
        waitpid(pid, &errorcode, 0);
        state = Done;
        auto x = std::move(onComplete);
        x(this);
    }

public:
    int pid = 0;
    std::thread thread;
    std::function<void(Task *)> onComplete;
    std::string filename;
};

Executor::Executor()
{
    for(size_t n = 0; n < 4; n++)
        activeTasks.push_back(nullptr);
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
    for(auto &c : activeTasks)
    {
        if(c && c->state != Task::Done)
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
    auto it = activeTasks.begin();
    for(auto &c : commands)
    {
        while(it != activeTasks.end() && *it)
            ++it;
        if(it == activeTasks.end())
            break;
        // TODO: take into account its relative load
        if(c->CanRun())
        {
            c->state = PendingCommand::Running;
            for(auto &o : c->outputs)
            {
                boost::filesystem::create_directories(o->path.parent_path());
            }
            *it = new Process(c->outputs[0]->path.filename().string(), c->commandToRun, "", [this, it, c](Task *t) {
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

    size_t w = 80 / activeTasks.size();
    size_t active = 0;
    for(auto &t : activeTasks)
        if(t)
            active++;

    printf("%zu concurrent tasks, %zu active, %zu commands left to run\n", activeTasks.size(), active, commands.size());
    for(auto &t : activeTasks)
    {
        std::string file;
        if(t)
        {
            file = ((Process *)t)->filename;
            if(file.size() > w - 3)
                file.resize(w - 3);
        }
        while(file.size() < w - 3)
            file.push_back(' ');
        printf("\033[1;37m[\033[0m%s\033[1;37m]\033[0m ", file.c_str());
    }
    printf("\r\033[1A");
    fflush(stdout);
}
