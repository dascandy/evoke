#include "Toolset.h"
#include "Component.h"
#include "PendingCommand.h"
#include "File.h"
#include "Project.h"
#include "view/filter.h"

static std::string getLibNameFor(Component& component) {
  return std::string("lib/") + component.root.string() + ".a";
}

static std::string getExeNameFor(Component& component) {
  return std::string("bin/") + component.root.string();
}

void UbuntuToolset::CreateCommandsFor(Project& project, Component& component) {
  boost::filesystem::path outputFolder = component.root;
  std::vector<File*> objects;

  std::set<Component*> deps = { &component };
  deps.insert(component.privDeps.begin(), component.privDeps.end());
  while (true) {
    std::set<Component*> newDeps = deps;
    for (auto d : deps) {
      newDeps.insert(d->pubDeps.begin(), d->pubDeps.end());
    }
    if (newDeps == deps) break;
    std::swap(deps, newDeps);
  }
  std::string includes;
  for (auto& d : deps) {
    includes += " -I" + d->root.string();
  }

  for (auto& f : filter(component.files, [&project](File*f){ return project.IsCompilationUnit(f->path.extension().string()); })) {
    boost::filesystem::path outputFile = std::string("obj") / outputFolder / (f->path.stem().string() + ".o");
    File* of = project.CreateFile(component, outputFile);
    PendingCommand* pc = new PendingCommand("g++ -c -o " + outputFile.string() + " " + component.root.string() + "/" + f->path.string() + includes);
    objects.push_back(of);
    pc->AddOutput(of);
    pc->AddInput(f);
    component.commands.push_back(pc);
  }
  if (!objects.empty()) {
    std::string command;
    boost::filesystem::path outputFile;
    PendingCommand* pc;
    if (component.type != "executable") {
      outputFile = getLibNameFor(component);
      command = "ar rcs " + outputFile.string();
    } else {
      outputFile = getExeNameFor(component);
      command = "g++ -o " + outputFile.string();

      // Gather all the private dependencies (and their dependencies) too now
      while (true) {
        std::set<Component*> newDeps = deps;
        for (auto d : deps) {
          newDeps.insert(d->pubDeps.begin(), d->pubDeps.end());
          newDeps.insert(d->privDeps.begin(), d->privDeps.end());
        }
        if (newDeps == deps) break;
        std::swap(deps, newDeps);
      }
      for (auto& d : deps) {
        command += " -l" + getLibNameFor(*d);
      }
    }
    for (auto& file : objects) {
      command += " " + file->path.string();
    }
    pc = new PendingCommand(command);
    File* libraryFile = project.CreateFile(component, outputFile);
    pc->AddOutput(libraryFile);
    for (auto& file : objects) {
      pc->AddInput(file);
    }
    component.commands.push_back(pc);
  }
}


