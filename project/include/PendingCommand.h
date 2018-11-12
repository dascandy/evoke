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

public:
    std::string commandToRun;
    enum State
    {
        Unknown,
        ToBeRun,
        Running,
        Done
    } state = Unknown;
    void SetResult(bool success);
    bool CanRun();
};

std::ostream &operator<<(std::ostream &os, const PendingCommand &);
