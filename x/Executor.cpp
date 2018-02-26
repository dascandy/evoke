#include "Executor.h"

struct Process {
public:
  Process(const std::string& cmd, const std::string& statefile, std::function<void()> onComplete) {
    int outfd[2];
    pipe2(outfd);
    if ((pid = fork()) == 0) {
      close(0);
      dup2(1, outfd[1]);
      dup2(2, outfd[1]);
      close(outfd[0]);
      execve();
      abort();
    }
  }
  ~Process() {

  }
private:
  void run() {
    
  }
  State state = Running;
  int pid;
  int errorcode;
  std::vector<char> outbuffer;
};




