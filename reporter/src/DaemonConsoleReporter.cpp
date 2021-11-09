#include "PendingCommand.h"
#include "Reporter.h"

#include <cstdio>
#include <iostream>
#include <string>

#ifndef _WIN32
#    include <signal.h>
#    include <sys/ioctl.h>
#    include <unistd.h>
#endif

static size_t screenHeight = 25;
static size_t screenWidth = 80;

bool hasFailure = false;
std::vector<std::shared_ptr<PendingCommand>> *commands = nullptr;

DaemonConsoleReporter* reporter = nullptr;
static void fetchDisplaySize()
{
#    ifdef TIOCGSIZE
    struct ttysize ts;
    ioctl(STDIN_FILENO, TIOCGSIZE, &ts);
    screenHeight = ts.ts_rows;
    screenWidth = ts.ts_cols;
#    elif defined(TIOCGWINSZ)
    struct winsize ts;
    ioctl(STDIN_FILENO, TIOCGWINSZ, &ts);
    screenHeight = ts.ws_row;
    screenWidth = ts.ws_col;
    // get a minimum set
    if(screenHeight <= 5)
        screenHeight = 3;
    if(screenWidth <= 0)
        screenWidth = 10;
#    endif /* TIOCGSIZE */
}

DaemonConsoleReporter::DaemonConsoleReporter()
{
    reporter = this;
    fetchDisplaySize();
#ifndef _WIN32
    signal(SIGWINCH, [](int) { fetchDisplaySize(); if (reporter) reporter->Redraw(); });
#endif
}

static const char red[] = "\033[1;31m";
static const char green[] = "\033[1;32m";
static const char yellow[] = "\033[1;33m";
static const char blue[] = "\033[1;34m";
static const char purple[] = "\033[1;35m";
static const char cyan[] = "\033[1;36m";
static const char white[] = "\033[1;37m";
static const char grey[] = "\033[0;37m";
static const char topleft[] = "\033[1;1H";
static const char clearscreen[] = "\033[2J";
static const char reset[] = "\033[m";

void printLines(size_t width, size_t& maxLines, const std::string& str) {
  std::string outline;
  size_t posWidth = 0;
  size_t maxWidth = maxLines * width - 1;
  // Pretend to add another newline at the end so the buffer flushes
  for (size_t n = 0; n < str.size() + 1; n++) {
    if (maxLines == 0) return;
    switch(n == str.size() ? '\n' : str[n]) {
    default: 
      if (str[n] == '\t') {
        outline += ' ';
      } else {
        outline += str[n]; 
      }
      posWidth++; 
      if (posWidth >= maxWidth) n = str.size() - 1;  // flush buffer, stop.
      break;
    case '\n': 
    case '\r': 
    {
      size_t linecount = (posWidth + width - 1) / width;
      if (linecount == 0) linecount++;
      maxLines -= linecount;
      maxWidth = maxLines * width - 1;
      std::cout << outline;
      if (maxLines == 0) 
        std::cout << std::flush;
      else 
        std::cout << '\n';
      outline.clear();
      posWidth = 0;
      break;
    }
    case '\b': break;
    case '\033':
      if (str.size() == n+1) break;
      // handle ansi sequence
      switch(str[n+1]) {
        case 'P': case 'X': case '^': case '_':
          // Ignore commands that take a string argument (and skip until end of string)
          while (str.size() > n+1 && (str[n] != 0x1b || str[n+1] != '\\')) n++;
          n++;
          break;
        case '[': // CSI
        {
          std::string csi;
          csi += str[n++];
          csi += str[n++];
          while (str.size() > n+1 && (str[n] < 0x40 || str[n] > 0x7E)) csi += str[n++];
          csi += str[n];
          switch(str[n]) {
            // Remove all cursor movement and stateful commands
            case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': 
            case 'S': case 'T': case 'f': case 'n': case 's': case 'u':
            break;
            default: outline += csi;
          }
          break;
        }
        case 'c': case 'N': case 'O':
          // Skip commands that switch character sets or reset the screen
          n++;
          break;
      }
      break;
    }
  }
}

void DaemonConsoleReporter::Redraw()
{
    std::cout << clearscreen << topleft << std::flush;

    size_t active = 0;
    for(auto &t : activeProcesses)
        if(t)
            active++;

    size_t commandsFailed = 0, commandCount = 0, commandsToBeRun = 0, commandsError = 0;
    if (commands) for (auto& command : *commands) {
        if (command->result->errorcode) commandsFailed++;
        if (command->state == PendingCommand::ToBeRun) commandsToBeRun++;
        if (command->state == PendingCommand::Error) commandsError++;
        commandCount++;
    }
    if (true) {
        if (active) {
            std::cout << yellow << " ¯\\_(ツ)_/¯ ";
        } else if (commandsFailed) {
            std::cout << red <<    "(╯°□°)╯︵┻━┻";
        } else {
            std::cout << green <<  " ᕕ ( ᐛ ) ᕗ  ";
        }
    } else {
        for (size_t n = 0; n < 4; n++) {
            std::cout << (commandsFailed ? red : blue) << "X" << (active ? yellow : commandsFailed ? red : green) << "X";
        }
    }
    std::cout << "  [ " << active << "/" << activeProcesses.size() << " active ][ " << commandsFailed << " failed ][ " << commandsError << " not built ][ " << commandCount << " pending ]\n";
    std::string s(screenWidth, '-');
    std::cout << blue << s << reset << std::flush;
    size_t linesLeft = screenHeight - 2;
    if (commands) {
        for (auto& command : *commands) {
            if (command->result->errorcode && !command->result->output.empty()) printLines(screenWidth, linesLeft, command->result->output);
        }
        for (auto& command : *commands) {
            if (!command->result->errorcode && !command->result->output.empty()) printLines(screenWidth, linesLeft, command->result->output);
        }
    }
}

void DaemonConsoleReporter::SetConcurrencyCount(size_t count)
{
    activeProcesses.resize(count);
    Redraw();
}

void DaemonConsoleReporter::SetRunningCommand(size_t channel, std::shared_ptr<PendingCommand> command)
{
    activeProcesses[channel] = command;
    Redraw();
}

void DaemonConsoleReporter::ReportCommand(size_t channel, std::shared_ptr<PendingCommand>)
{
    activeProcesses[channel] = nullptr;
    Redraw();
}

void DaemonConsoleReporter::ReportCommandQueue(std::vector<std::shared_ptr<PendingCommand>>& allCommands) 
{
    commands = &allCommands;
    Redraw();
}


