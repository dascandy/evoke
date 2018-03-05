#include "Toolset.h"
#include "Component.h"
#include "PendingCommand.h"
#include "File.h"
#include "Project.h"
#include "view/filter.h"

static std::string getLibNameFor(Component& component) {
  // TODO: change commponent to dotted string before making
  return "lib" + component.root.string() + ".a";
}

static std::string getExeNameFor(Component& component) {
  return component.root.string();
}

std::vector<Component*> GetTransitiveAllDeps(Component& c) {
  
}

std::vector<Component*> GetTransitivePubDeps(Component& c) {

}

void UbuntuToolset::CreateCommandsFor(Project& project, Component& component) {
  boost::filesystem::path outputFolder = component.root;
  std::vector<File*> objects;

  std::vector<Component*> deps = { &component };
  deps.insert(component.privDeps.begin(), component.privDeps.end());
  for (size_t n = 0; n < deps.size(); n++) { // not range based as we will modify deps
    for (auto& id : d->pubDeps) {
      if (deps.find(id) == deps.end()) deps.push_back(&id);
    }
  }
  deps.erase(deps.find(&component));
  std::string includes;
  for (auto& d : deps) {
    includes += " -I" + d->root.string();
  }

  for (auto& f : filter(component.files, [&project](File*f){ return project.IsCompilationUnit(f->path.extension().string()); })) {
    boost::filesystem::path outputFile = std::string("obj") / outputFolder / (f->path.stem().string() + ".o");
    File* of = project.CreateFile(component, outputFile);
    PendingCommand* pc = new PendingCommand("g++ -c -o " + outputFile.string() + " " + f->path.string() + includes);
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
      outputFile = "lib/" + getLibNameFor(component);
      command = "ar rcs " + outputFile.string();
      for (auto& file : objects) {
        command += " " + file->path.string();
      }
    } else {
      outputFile = "bin/" + getExeNameFor(component);
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
      if (deps.find(&component) != deps.end()) 
        deps.erase(deps.find(&component));
      command += " -Llib";
      for (auto& file : objects) {
        command += " " + file->path.string();
      }
      for (auto& d : deps) {
        command += " -l" + d->root.string();
      }
    }
    pc = new PendingCommand(command);
    File* libraryFile = project.CreateFile(component, outputFile);
    pc->AddOutput(libraryFile);
    for (auto& file : objects) {
      pc->AddInput(file);
    }
    if (component.type == "executable") {
      for (auto& d : deps) {
        pc->AddInput(project.CreateFile(*d, "lib/" + getLibNameFor(*d)));
      }
    }
    component.commands.push_back(pc);
  }
}


