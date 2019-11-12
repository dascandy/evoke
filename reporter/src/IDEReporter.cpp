#include "PendingCommand.h"
#include "Reporter.h"

#include <iostream>

void IDEReporter::SetConcurrencyCount(size_t count)
{
    std::cout << "EVOKE CONCURRENCY " << count << "\n" << std::flush;
}

void IDEReporter::SetRunningCommand(size_t channelno, std::shared_ptr<PendingCommand> command)
{
    std::cout << "EVOKE RUNNINGCOMMAND CHANNEL " << channelno << " COMMAND " << (command ? command->commandToRun : "NULL") << "\n" << std::flush;
}

void IDEReporter::ReportCommand(size_t , std::shared_ptr<PendingCommand> command)
{
    std::cout << "EVOKE COMPLETION " << command->commandToRun << " ERROR " << command->errorcode << " RESULT\n" << command->output << "\n" << std::flush;
}
