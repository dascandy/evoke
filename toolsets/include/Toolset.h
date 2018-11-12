#pragma once

#include <memory>

struct Component;
class Project;

struct Toolset
{
    virtual ~Toolset()
    {
    }
    virtual void CreateCommandsFor(Project &project) = 0;
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
};

struct WindowsToolset : public Toolset
{
    void CreateCommandsFor(Project &project) override;
};

std::unique_ptr<Toolset> GetToolsetByName(const std::string &name);
