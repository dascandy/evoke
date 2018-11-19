#include "Reporter.h"
#include "PendingCommand.h"
#include <string>
#include <cstdio>
#include <iostream>
#include <iostream>

#ifndef _WIN32
#include <sys/ioctl.h>
#include <unistd.h>
#include <signal.h>
#endif


static size_t screenWidth = 80;

#ifndef _WIN32
void fetchDisplaySize() {
#ifdef TIOCGSIZE
    struct ttysize ts;
    ioctl(STDIN_FILENO, TIOCGSIZE, &ts);
    screenWidth = ts.ts_cols;
#elif defined(TIOCGWINSZ)
    struct winsize ts;
    ioctl(STDIN_FILENO, TIOCGWINSZ, &ts);
    screenWidth = ts.ws_col;
#endif /* TIOCGSIZE */
}
#endif

ConsoleReporter::ConsoleReporter() {
#ifndef _WIN32
    fetchDisplaySize();
    signal(SIGWINCH, [](int){ fetchDisplaySize(); });
#endif
}

void ConsoleReporter::Redraw() {
    size_t w = screenWidth / activeProcesses.size();
    size_t active = 0;
    for(auto &t : activeProcesses)
        if(t)
            active++;
    
    std::cout << activeProcesses.size() << " concurrent tasks, " << active << " active\n";
    if(w == 0)
    {
        // If you have more cores than horizontal characters, we can't display one task per character. Instead make the horizontal line a "usage" bar representing proportional task use.
        size_t activeCount = 0;
        for(auto &t : activeProcesses)
        {
            if(t)
                activeCount++;
        }
        std::cout << std::string((screenWidth - 1) * activeCount / activeProcesses.size(), '*') << std::string(screenWidth - (screenWidth - 1) * activeCount / activeProcesses.size() - 1, ' ');
    }
    else
    {
        for(auto &t : activeProcesses)
        {
            if(w >= 10)
            {
                std::string file;
                if(t)
                {
                    file = t->inputs[0]->path.generic_string();
                    if(file.size() > w - 2)
                        file.resize(w - 2);
                }
                while(file.size() < w - 2)
                    file.push_back(' ');
                std::cout << "\033[1;37m[\033[0m" << file << "\033[1;37m]\033[0m";
            }
            else if(w >= 3)
            {
                std::cout << "\033[1;37m[\033[0m" << (t ? "*" : "_") << "\033[1;37m]\033[0m";
            }
            else if(w >= 1)
            {
                std::cout << "\033[1;37m" << (t ? "*" : "_") << "\033[0m";
            }
        }
    }
    std::cout << "\r\033[1A" << std::flush;
}

void ConsoleReporter::SetConcurrencyCount(size_t count) {
    activeProcesses.resize(count);
    Redraw();
}

void ConsoleReporter::SetRunningCommand(size_t channel, PendingCommand* command) {
    activeProcesses[channel] = command;
    Redraw();
}

void ConsoleReporter::ReportFailure(PendingCommand* , int , const std::string& errors) {
    // Display error
    std::cout << "\n\n" << errors << "\n\n";
    Redraw();
}


