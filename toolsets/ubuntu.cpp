#include "Toolset.h"
#include "Component.h"
#include "PendingCommand.h"
#include "File.h"
#include "Project.h"
#include "view/filter.h"

void UbuntuToolset::CreateCommandsFor(Project& project, Component& component) {
  if (!objects.empty()) {
    boost::filesystem::path outputFolder = component.root;
    std::vector<File*> objects;
    // TODO: find include paths
    for (auto& f : filter(component.files, [&project](File*f){ return project.IsCompilationUnit(f->path.extension().string()); })) {
      boost::filesystem::path outputFile = std::string("obj") / outputFolder / (f->path.stem().string() + ".o");
      File* of = project.CreateFile(component, outputFile);
      PendingCommand* pc = new PendingCommand("g++ -c -o " + outputFile.string() + " " + f->path.string());
      objects.push_back(of);
      pc->AddOutput(of);
      pc->AddInput(f);
      component.commands.push_back(pc);
    }
    std::string command;
    boost::filesystem::path outputFile;
    PendingCommand* pc;
    if (component.type != "executable") {
      outputFile = std::string("lib/") + outputFolder.string() + ".a";
      command = "ar rcs " + outputFile.string();
      for (auto& file : objects) {
        command += " " + file->path.string();
      }
      pc = new PendingCommand(command);
    } else {
      // TODO: convert all deps to link statements
      outputFile = std::string("bin/") + outputFolder.string();
      command = "g++ -o " + outputFile.string();
      for (auto& file : objects) {
        command += " " + file->path.string();
      }
      pc = new PendingCommand(command);
    }
    File* libraryFile = project.CreateFile(component, outputFile);
    pc->AddOutput(libraryFile);
    for (auto& file : objects) {
      pc->AddInput(file);
    }
    component.commands.push_back(pc);
  }
}


