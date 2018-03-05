#include "Executor.h"
#include <functional>
#include <thread>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <view/split.h>
#include <cstring>
#include "PendingCommand.h"

struct Process : public Task {
public:
  Process(const std::string& cmd, const std::string& statefile, std::function<void()> onComplete) 
  : onComplete(onComplete)
  {
    int outfd[2];
    pipe(outfd);
    if ((pid = fork()) == 0) {
      close(0);
      dup2(1, outfd[1]);
      dup2(2, outfd[1]);
      close(outfd[0]);
      std::vector<char*> argv;
      size_t start = 0, end = cmd.find_first_of(" ");
      while (end != cmd.npos) {
        argv.push_back(strndup(cmd.data() + start, end - start));
        start = cmd.find_first_not_of(" ", end);
        end = cmd.find_first_of(" ", start);
      }
      argv.push_back(strdup(cmd.data() + start));
      argv.push_back(nullptr);
      int i = 0;
      for (auto& p : argv) {
        fprintf(stderr, "argv[%d] = \"%s\"\n", i++, p);
      }
      execvp(argv[0], argv.data());
      abort();
    }
    close(outfd[1]);
    thread = std::thread([this, fd = outfd[0]]{ run(fd); });
  }
  ~Process() {
    thread.join();
  }
private:
  void run(int fd) {
    char buffer[1024];
    while (true) {
      int bread = read(fd, buffer, sizeof(buffer));
      if (bread == 0) break;
      else if (bread < 0) {
        if (errno == EINTR) continue;
      }
      outbuffer.insert(outbuffer.end(), buffer, buffer + bread);
    }
    waitpid(pid, &errorcode, 0);
    state = Done;
    onComplete();
  }
  int pid = 0;
  std::thread thread;
  std::function<void()> onComplete;
};

void Executor::Run(PendingCommand* cmd) {
  std::lock_guard<std::mutex> l(m);
  commands.push_back(cmd);
}

bool Executor::Busy() {
  // What if they just all finished and just haven't started another yet?
  std::lock_guard<std::mutex> l(m);
  for (auto& c : activeTasks) {
    if (c->state != Task::Done) return true;
  }
  return false;
}

void Executor::Start() {
  RunMoreCommands();
}

void Executor::RunMoreCommands() {
  std::lock_guard<std::mutex> l(m);
  for (auto& c : commands) {
    // TODO: take into account its relative load
    if (c->CanRun()) {
      printf("Can run %s\n", c->commandToRun.c_str());
      activeTasks.push_back(new Process(c->commandToRun, "", [this](){ /*RunMoreCommands();*/ }));
    } else
      printf("Cannot run %s\n", c->commandToRun.c_str());
  }
}


