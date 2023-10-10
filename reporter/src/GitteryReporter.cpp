#include "PendingCommand.h"
#include "Reporter.h"

#include <cstdio>
#include <iostream>
#include <string>

void GitteryReporter::ReportCommand(size_t , std::shared_ptr<PendingCommand> cmd)
{
  std::cerr << cmd->result->output;
}
