#include "Reporter.h"

#include <iostream>
#include <string>
#ifdef _WIN32
int isatty(int) { return 0; }
#else
#include <unistd.h>
#endif

std::unique_ptr<Reporter> Reporter::Get(const std::string &name)
{
    if(name == "simple")
    {
        return std::make_unique<SimpleReporter>();
    }
    else if(name == "console")
    {
        return std::make_unique<ConsoleReporter>();
    }
#ifdef DAEMON_SUPPORT
    else if(name == "daemon")
    {
        if(isatty(0) && isatty(1))
        {
            return std::make_unique<DaemonConsoleReporter>();
        }
        else
        {
            return std::make_unique<IDEReporter>();
        }
    }
#endif
    else if(name == "guess")
    {
        if(isatty(0) && isatty(1))
        {
            return std::make_unique<ConsoleReporter>();
        }
        else
        {
            return std::make_unique<SimpleReporter>();
        }
    }
    else
    {
        std::cout << "Unknown reporter: " << name << "\n";
        return std::make_unique<SimpleReporter>();
    }
}
