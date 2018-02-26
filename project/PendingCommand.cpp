#include "PendingCommand.h"
#include "File.h"

PendingCommand::PendingCommand(const std::string& command) 
: commandToRun(command)
{
}

void PendingCommand::AddInput(File* input) {
  inputs.push_back(input);
  input->listeners.push_back(this);
  Check();
}

void PendingCommand::AddOutput(File* output) {
  if (output->generator) 
    throw std::runtime_error("Already found a command to create this output file");

  output->generator = this;
  outputs.push_back(output);
  Check();
}

void PendingCommand::Check() {
  if (outputs.empty()) return;
  if (state == PendingCommand::ToBeRun) return;
  std::time_t oldestOutput = outputs[0]->lastwrite();
  for (auto& out : outputs) {
    if (out->lastwrite() < oldestOutput) oldestOutput = out->lastwrite();
  }
  for (auto& in : inputs) {
    if (in->lastwrite() > oldestOutput) {
      state = PendingCommand::ToBeRun;
      return;
    }
    if (in->generator) {
      in->generator->Check();
      if (in->generator->state == PendingCommand::ToBeRun) {
        state = PendingCommand::ToBeRun;
      }
    }
  }
  state = PendingCommand::Done;
}

std::ostream& operator<<(std::ostream& os, const PendingCommand& pc) {
  os << pc.commandToRun << " state=";
  switch(pc.state) {
    case PendingCommand::Unknown: os << "unknown"; break;
    case PendingCommand::ToBeRun: os << "to be run"; break;
    case PendingCommand::Running: os << "running"; break;
    case PendingCommand::Done: os << "done"; break;
  }
  return os;
}


