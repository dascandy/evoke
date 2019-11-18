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
        Done,
        Depfail
    } state = Unknown;
    void SetResult(int errorcode, std::string messages);
    bool CanRun();
    std::string output;
    int errorcode = 0;
};

std::ostream &operator<<(std::ostream &os, const PendingCommand &);
