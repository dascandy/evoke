#pragma once

#include "File.h"

#include <ostream>
#include <string>
#include <vector>
#include <array>

class Toolset;

struct PendingCommand
{
public:
    struct FileRecord {
      std::array<uint8_t, 64> toolsetHash;
      std::array<uint8_t, 64> tuHash;
      double timeEstimate;
      uint64_t spaceNeeded;
      std::string output;
      uint32_t measurementCount;
      int errorcode;
    };

    PendingCommand(std::array<uint8_t, 64> toolsetHash, const std::string &command);
    void AddInput(File *input);
    void AddOutput(File *output);
    std::vector<File *> inputs;
    std::vector<File *> outputs;
    void Check();
    float timeToComplete() {
        if (result->measurementCount > 0)
            return result->timeEstimate / result->measurementCount + longestChildCommand;
        return result->timeEstimate + longestChildCommand;
    }
    uint64_t memoryUse() {
        if (result->measurementCount > 0)
            return result->spaceNeeded / result->measurementCount;
        return result->spaceNeeded;
    }
    void addChildCommand(float length) {
        if (length <= longestChildCommand) 
          return;

        longestChildCommand = length;
        for (auto& f : inputs) {
            if (f->generator)
                f->generator->addChildCommand(timeToComplete());
        }
    }
public:
    float longestChildCommand = 0;
    std::string commandToRun;
    std::array<uint8_t, 64> toolsetHash;
    enum State
    {
        Unknown,
        ToBeRun,
        Running,
        Done,
        Error
    } state = Unknown;
    void SetResult(FileRecord newFileRecord);
    bool ReadyToStart();
    struct Result {
        std::string output;
        uint32_t errorcode = 0;
        uint32_t measurementCount = 0;
        double timeEstimate = 1; // assumption: 1 second.
        uint64_t spaceNeeded = 1 << 30; // assumption: 1 GB of memory use
    };
    FileRecord* result = nullptr;
};

std::ostream &operator<<(std::ostream &os, const PendingCommand &);
void SaveCommandResultDb();
