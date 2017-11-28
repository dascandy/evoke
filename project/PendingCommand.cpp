#include "PendingCommand.h"
#include "File.h"

PendingCommand::PendingCommand(const std::string& command) 
: commandToRun(command)
{
}

void PendingCommand::AddInput(File* input) {
  inputs.push_back(input);
  input->listeners.push_back(this);
}

void PendingCommand::AddOutput(File* output) {
  if (output->generator) 
    throw std::runtime_error("Already found a command to create this output file");

  output->generator = this;
  output->state = File::ToRebuild;
  outputs.push_back(output);
}

void PendingCommand::SignalRecheck() {
  bool readyToBuild = true;
  for (auto& i : inputs) {
    if (i->state != File::Source &&
        i->state != File::Done) {
      readyToBuild = false;
    }
  }
  if (readyToBuild)
    TriggerRebuild();
}

void PendingCommand::TriggerRebuild() {

}

