#pragma once

class Component;
class Project;

struct Toolset {
  virtual void CreateCommandsFor(Project& project, Component& component) = 0;
};


struct UbuntuToolset : public Toolset {
    void CreateCommandsFor(Project& project, Component& component) override;
};
/*

1. generate sources from inputs
2. compile sources to object files
3. link object files to static/shared library file

*/
