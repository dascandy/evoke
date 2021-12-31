#include "PendingCommand.h"
#include "File.h"
#include <fstream>
#include <map>

struct FileEntry {
  std::array<uint8_t, 64> toolsetHash;
  std::array<uint8_t, 64> tuHash;
  double timeEstimate;
  uint64_t spaceNeeded;
  uint32_t measurementCount;
  uint32_t mainOutputNameLength;
  uint32_t outputSize;
  uint8_t errorCode;
};
static const constexpr uint32_t evoke_magic = 0x45564F4B;
static const constexpr uint32_t evoke_version = 0x00010000;

struct DbHeader {
  uint32_t magic = evoke_magic;
  uint32_t version = evoke_version;
  uint32_t entryCount = 0;
};

struct CommandResultDb {
  std::map<std::string, PendingCommand::FileRecord> knownFiles;
  CommandResultDb() {
    std::ifstream in(".evoke.db");
    DbHeader header;
    in.read((char*)&header, sizeof(header));
    if (!in.good() || header.magic != evoke_magic || header.version != evoke_version) return;
    for (size_t n = 0; n < header.entryCount; n++) {
      FileEntry fe;
      in.read((char*)&fe, sizeof(fe));
      if (!in.good()) return;
      std::string name;
      name.resize(fe.mainOutputNameLength);
      in.read(name.data(), name.size());
      if (!in.good()) return;
      std::string output;
      output.resize(fe.outputSize);
      in.read(output.data(), output.size());
      if (!in.good()) return;
      knownFiles[std::move(name)] = PendingCommand::FileRecord{fe.toolsetHash, fe.tuHash, fe.timeEstimate, fe.spaceNeeded, std::move(output), fe.measurementCount, fe.errorCode};
    }
  }
  void Read(PendingCommand& command) {
    if (command.outputs.empty()) {
      command.result = &knownFiles[command.commandToRun + "#"];
    } else {
      command.result = &knownFiles[command.outputs[0]->path.string()];
    }
  }
  void Save() {
    std::ofstream out(".evoke.db.new");
    DbHeader header;
    header.entryCount = static_cast<std::uint32_t>(knownFiles.size());
    out.write((const char*)&header, sizeof(header));
    for (auto& [name, record] : knownFiles) {
      FileEntry fe{record.toolsetHash, record.tuHash, record.timeEstimate, record.spaceNeeded, record.measurementCount, static_cast<uint32_t>(name.size()), static_cast<uint32_t>(record.output.size()), (uint8_t)record.errorcode};
      out.write((const char*)&fe, sizeof(fe));
      out.write(name.data(), name.size());
      out.write(record.output.data(), record.output.size());
    }
    out.close();
    fs::rename(".evoke.db.new", ".evoke.db");
  }
  ~CommandResultDb() {
    Save();
  }
};

static CommandResultDb& ResultDb() {
  static CommandResultDb db;
  return db;
}

void SaveCommandResultDb() {
  ResultDb().Save();
}

PendingCommand::PendingCommand(std::array<uint8_t, 64> toolsetHash, const std::string &command)
: commandToRun(command)
, toolsetHash(toolsetHash)
{
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
    if (outputs.size() == 1) {
        ResultDb().Read(*this);
    }
}

// Update the command status to either being up to date, being out of date, or being in a state where it cannot run
void PendingCommand::Check()
{
    if (outputs.size() == 0) {
        ResultDb().Read(*this);
    }
    if (state != PendingCommand::Unknown) 
        return;

    for(auto &in : inputs) {
        if (in->generator) {
            in->generator->Check();
        }
    }

    // If any input file is in error, or does not exist without any generator, then this cannot build
    for(auto &in : inputs) {
        if ((in->state == File::Error) ||
            (not in->generator && not in->Exists())) {
            printf("Marking cmd as error\n");
            printf("cmd is %s\n", commandToRun.c_str());
            printf("input failing is %s\n", in->path.c_str());
            state = PendingCommand::Error;
            for(auto &o : outputs)
            {
                o->state = File::Error;
                for (auto &d : o->listeners) {
                    d->Check();
                }
            }
            return;
        }
    }

    state = PendingCommand::Done;

    // If any input file needs to be built itself then this one needs to be subsequently built
    for(auto &in : inputs) {
        if (in->generator && in->generator->state == PendingCommand::ToBeRun) {
            state = PendingCommand::ToBeRun;
        }
    }

    // If any output does not exist, we need to rerun this command (even if the other results are up to date)
    for(auto &out : outputs)
    {
        if(not out->Exists()) {
            state = PendingCommand::ToBeRun;
        }
    }

    // If the last build output was with a different toolset or input, rebuild
    if (toolsetHash != result->toolsetHash) {
        printf("Toolset hash changed %s\n", commandToRun.c_str());
        state = PendingCommand::ToBeRun;
    }

    std::array<uint8_t, 64> hash = {};
    for (auto& i : inputs) {
        for (size_t n = 0; n < 64; n++) 
            hash[n] ^= i->hash[n];
    }
    if (result->tuHash != hash) {
        for (size_t n = 0; n < 64; n++) {
            printf("%02X", hash[n]);
        }
        printf("\n");
        for (size_t n = 0; n < 64; n++) {
            printf("%02X", result->tuHash[n]);
        }
        printf("\n");
        printf("TU hash changed: %s\n", commandToRun.c_str());
        state = PendingCommand::ToBeRun;
    }

    if (state == PendingCommand::Done) {
        for(auto &o : outputs) {
            o->state = File::Done;
        }
    } else {
        // Notify any dependants that may not be checked that they should update anyway
        for(auto &o : outputs) {
            o->state = File::ToRebuild;
            for(auto &d : o->listeners) {
                d->Check();
            }
        }
    }
}

void PendingCommand::SetResult(PendingCommand::FileRecord newResult)
{
    *result = newResult;
    if (newResult.errorcode != 0) {
        state = PendingCommand::Error;
        for(auto &o : outputs)
        {
            o->state = File::Error;
            for (auto &d : o->listeners) {
                d->Check();
            }
        }
    } else
        state = PendingCommand::Unknown;
    Check();
}

// may become runnable, precondition-fail, already running, already finished
bool PendingCommand::ReadyToStart()
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
    case PendingCommand::Error:
        os << "Error";
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
