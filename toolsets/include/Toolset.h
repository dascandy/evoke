#pragma once

#include "globaloptions.h"

#include <memory>
#include <string>
#include <vector>

struct Component;
class Project;

struct Toolset
{
    virtual ~Toolset()
    {
    }
    virtual void CreateCommandsFor(Project &project) = 0;
    virtual GlobalOptions ParseGeneralOptions(const std::string &options);
};

struct AndroidToolset : public Toolset
{
    void CreateCommandsFor(Project &project) override;
};

struct ClangToolset : public Toolset
{
    void CreateCommandsFor(Project &project) override;
};

struct UbuntuToolset : public Toolset
{
    void CreateCommandsFor(Project &project) override;
    GlobalOptions ParseGeneralOptions(const std::string &options) override;
};

struct WindowsToolset : public Toolset
{
    void CreateCommandsFor(Project &project) override;
};

std::unique_ptr<Toolset> GetToolsetByName(const std::string &name);
