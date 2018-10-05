#pragma once

#include <memory>

struct Component;
class Project;

struct Toolset {
  virtual void CreateCommandsFor(Project& project, Component& component) = 0;
};

struct AndroidToolset : public Toolset {
    void CreateCommandsFor(Project& project, Component& component) override;
};

struct UbuntuToolset : public Toolset {
    void CreateCommandsFor(Project& project, Component& component) override;
};

struct WindowsToolset : public Toolset {
    void CreateCommandsFor(Project& project, Component& component) override;
};

std::unique_ptr<Toolset> GetToolsetByName(const std::string& name);


