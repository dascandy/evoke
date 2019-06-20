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
    fetchDisplaySize();
#ifndef _WIN32
    signal(SIGWINCH, [](int) { fetchDisplaySize(); });
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

void DaemonConsoleReporter::Redraw()
{
    std::cout << clearscreen << topleft << std::flush;

    size_t active = 0;
    for(auto &t : activeProcesses)
        if(t)
            active++;

    size_t commandsFailed = 0, commandCount = 0, commandsToBeRun = 0, commandsDepfail = 0;
    if (commands) for (auto& command : *commands) {
        if (command->errorcode) commandsFailed++;
        if (command->state == PendingCommand::ToBeRun) commandsToBeRun++;
        if (command->state == PendingCommand::Depfail) commandsDepfail++;
        commandCount++;
    }
    std::cout << (commandsFailed ? red : blue) << "X" << (active ? yellow : commandsFailed ? red : green) << "X";
    std::cout << (commandsFailed ? red : blue) << "X" << (active ? yellow : commandsFailed ? red : green) << "X";
    std::cout << (commandsFailed ? red : blue) << "X" << (active ? yellow : commandsFailed ? red : green) << "X";
    std::cout << (commandsFailed ? red : blue) << "X" << (active ? yellow : commandsFailed ? red : green) << "X";
    std::cout << "  [ " << active << "/" << activeProcesses.size() << " active ][ " << commandsFailed << " failed ][ " << commandsDepfail << " not built ][ " << commandCount << " total ]\n";
    std::string s(screenWidth, '-');
    std::cout << blue << s << reset << std::flush;
    if (commands) {
        for (auto& command : *commands) {
            if (command->errorcode && !command->output.empty()) {
                std::cout << command->output << "\n" << std::flush;
            }
        }
        for (auto& command : *commands) {
            if (!command->errorcode && !command->output.empty()) {
                std::cout << command->output << "\n" << std::flush;
            }
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

void DaemonConsoleReporter::ReportCommand(size_t channel, std::shared_ptr<PendingCommand> cmd)
{
    activeProcesses[channel] = nullptr;
    Redraw();
}

void DaemonConsoleReporter::ReportCommandQueue(std::vector<std::shared_ptr<PendingCommand>>& allCommands) 
{
    commands = &allCommands;
    Redraw();
}


