#pragma once

#include "File.h"

#include <ostream>
#include <string>
#include <vector>

struct PendingCommand
{
public:
    PendingCommand(const std::string &command);
    void AddInput(File *input);
    void AddOutput(File *output);
    std::vector<File *> inputs;
    std::vector<File *> outputs;
    void Check();
    float timeToComplete() {
        return result->timeEstimate / result->measurementCount + longestChildCommand;
    }
    uint64_t memoryUse() {
        return result->spaceNeeded / result->measurementCount;
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
    enum State
    {
        Unknown,
        ToBeRun,
        Running,
        Done,
        Depfail
    } state = Unknown;
    void SetResult(int errorcode, std::string messages, double timeTaken, uint64_t spaceUsed);
    bool CanRun();
    struct Result {
        std::string output;
        uint32_t errorcode = 0;
        uint32_t measurementCount = 0;
        double timeEstimate = 1; // assumption: 1 second.
        uint64_t spaceNeeded = 1 << 30; // assumption: 1 GB of memory use
    };
    Result* result = nullptr;
};

std::ostream &operator<<(std::ostream &os, const PendingCommand &);
