#pragma once

struct Component;
class Project;

struct Toolset {
  virtual void CreateCommandsFor(Project& project, Component& component) = 0;
};

struct UbuntuToolset : public Toolset {
    void CreateCommandsFor(Project& project, Component& component) override;
};


