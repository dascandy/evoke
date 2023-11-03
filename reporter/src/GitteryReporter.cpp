#include "PendingCommand.h"
#include "Reporter.h"

#include <cstdio>
#include <iostream>
#include <string>

GitteryReporter::GitteryReporter() {
  std::cerr << "[ { \"version\": \"1.0\" }";
}

GitteryReporter::~GitteryReporter() {
  std::cerr << " ]";
}

void GitteryReporter::ReportCommand(size_t , std::shared_ptr<PendingCommand> cmd)
{
  std::cerr << ", { \"command\": \"" << cmd->commandToRun << "\", \"output\": ";
  std::cerr << cmd->result->output;
  std::cerr << " }";
}


