#include "PendingCommand.h"
#include "File.h"

struct CommandResultDb {
  struct result_on_disk {
    uint32_t errorcode, measurementCount;
    double timeEstimate;
    uint64_t spaceNeeded;
    uint32_t commandSize, outputSize;
  };
  CommandResultDb() {
    std::ifstream in(".evoke.db");
    while (in.good()) {
      result_on_disk r;
      in.read((char*)&r, sizeof(r));
      std::string command, output;
      command.resize(r.commandSize);
      output.resize(r.outputSize);
      in.read((char*)command.data(), command.size());
      in.read((char*)output.data(), output.size());
      results[command] = PendingCommand::Result{output, r.errorcode, r.measurementCount, r.timeEstimate, r.spaceNeeded};
    }
  }
  void Read(PendingCommand& command) {
    command.result = &results[command.commandToRun];
  }
  void Save() {
    std::ofstream out(".evoke.db");
    for (auto& [command, r] : results) {
      if (r.output.empty() && r.measurementCount == 0) continue;
      result_on_disk res{ (uint32_t)r.errorcode, (uint32_t)r.measurementCount, r.timeEstimate, r.spaceNeeded, (uint32_t)command.size(), (uint32_t)r.output.size() };
      out.write((const char*)&res, sizeof(res));
      out.write(command.data(), command.size());
      out.write(r.output.data(), r.output.size());
    }
  }
  ~CommandResultDb() {
    Save();
  }
  std::map<std::string, PendingCommand::Result> results;
};

static CommandResultDb& ResultDb() {
  static CommandResultDb db;
  return db;
}

void SaveCommandResultDb() {
  ResultDb().Save();
}

PendingCommand::PendingCommand(const std::string &command) :
    commandToRun(command)
{
  ResultDb().Read(*this);
}

void PendingCommand::AddInput(File *input)
{
    inputs.push_back(input);
    input->listeners.push_back(this);
}

void PendingCommand::AddOutput(File *output)
{
    if(output->generator)
    {
        fprintf(stderr, "Multiple rules define %s\n", output->path.string().c_str());
        return;
    }

    output->generator = this;
    output->state = File::Unknown;
    outputs.push_back(output);
}

void PendingCommand::Check()
{
    if(state == PendingCommand::ToBeRun) 
        return;

    enum ShouldRebuildResult {
      RebuildNeeded,
      RebuildNotNeeded,
      ErrorState,
    };
    auto result = [&]{
        if(outputs.empty() || state == PendingCommand::ToBeRun) 
            return RebuildNeeded;

        bool missingOutput = false;
        std::time_t oldestOutput = outputs[0]->lastwrite();
        for(auto &out : outputs)
        {
            if(out->lastwrite() == 0)
                missingOutput = true;
            else if(out->lastwrite() < oldestOutput)
                oldestOutput = out->lastwrite();
        }

        for(auto &in : inputs)
        {
            if (in->state == File::Error)
                return ErrorState;
            if(in->lastwrite() > oldestOutput)
                return RebuildNeeded;
            if(in->generator)
            {
                in->generator->Check();
                if(in->generator->state == PendingCommand::ToBeRun)
                    return RebuildNeeded;
            }
        }
        if(missingOutput)
            return RebuildNeeded;
        return RebuildNotNeeded;
    }();

    switch (result) {
        case ErrorState:
            state = PendingCommand::Depfail;
            for(auto &o : outputs)
            {
                o->state = File::Error;
            }
            break;
        case RebuildNeeded:
            state = PendingCommand::ToBeRun;
            for(auto &o : outputs)
            {
                o->state = File::ToRebuild;
                for(auto &d : o->listeners)
                    d->Check();
            }
            break;
        case RebuildNotNeeded:
            for(auto &o : outputs)
            {
                o->state = File::Done;
            }
            break;
    }
}

void PendingCommand::SetResult(int errorcode, std::string messages, double timeTaken, uint64_t spaceUsed)
{
    result->errorcode = errorcode;
    result->output = std::move(messages);

    if (errorcode == 0 && timeTaken > 0 && spaceUsed > 0) {
        if (result->measurementCount == 10) {
          result->timeEstimate *= 0.9;
          result->spaceNeeded *= 0.9;
          result->measurementCount--;
        }
        result->timeEstimate += timeTaken;
        result->spaceNeeded += spaceUsed;
        result->measurementCount++;
    }

    state = PendingCommand::Done;
    for(auto &o : outputs)
    {
        o->state = (errorcode ? File::Error : File::Done);
    }
}

// may become runnable, precondition-fail, already running, already finished
bool PendingCommand::CanRun()
{
    if(state != PendingCommand::ToBeRun)
        return false;
    for(auto &in : inputs)
    {
        if(in->state != File::Unknown && in->state != File::Source && in->state != File::Done)
        {
            return false;
        }
    }
    return true;
}

std::ostream &operator<<(std::ostream &os, const PendingCommand &pc)
{
    os << pc.commandToRun << " state=";
    switch(pc.state)
    {
    case PendingCommand::Unknown:
        os << "unknown";
        break;
    case PendingCommand::ToBeRun:
        os << "to be run";
        break;
    case PendingCommand::Depfail:
        os << "Depfail";
        break;
    case PendingCommand::Running:
        os << "running";
        break;
    case PendingCommand::Done:
        os << "done";
        break;
    }
    os << "\n  depends on [ ";
    for(auto &in : pc.inputs)
    {
        os << in->path.string() << " ";
    }
    os << "]";
    return os;
}
