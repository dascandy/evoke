#include "Toolset.h"
#include "Component.h"
#include "PendingCommand.h"
#include "File.h"
#include "Project.h"
#include "view/filter.h"

void UbuntuToolset::CreateCommandsFor(Project& project, Component& component) {
  boost::filesystem::path outputFolder = component.root;
  std::vector<File*> objects;
  for (auto& f : filter(component.files, [&project](File*f){ return project.IsCompilationUnit(f->path.extension().string()); })) {
    PendingCommand* pc = new PendingCommand;
    boost::filesystem::path outputFile = outputFolder / (f->path.stem().string() + ".o");
    File* of = project.CreateFile(component, outputFile);
    objects.push_back(of);
    pc->outputs.push_back(of);
    pc->inputs.push_back(f);
    pc->commandToRun = "g++ -c -o " + outputFile.string() + " " + f->path.string();
    component.commands.push_back(pc);
  }
  if (!objects.empty()) {
    PendingCommand* pc = new PendingCommand;
    pc->inputs = objects;
    if (component.type != "executable") {
      boost::filesystem::path outputFile = outputFolder.string() + ".a";
      File* libraryFile = project.CreateFile(component, outputFile);
      pc->outputs.push_back(libraryFile);
      pc->commandToRun = "ar rcs " + libraryFile->path.string();
      for (auto& file : objects) {
        pc->commandToRun += " " + file->path.string();
      }
    } else {
      boost::filesystem::path outputFile = outputFolder.string() + ".exe";
      File* libraryFile = project.CreateFile(component, outputFile);
      pc->outputs.push_back(libraryFile);
      pc->commandToRun = "g++ -o " + libraryFile->path.string();
      for (auto& file : objects) {
        pc->commandToRun += " " + file->path.string();
      }
    }
    component.commands.push_back(pc);
  }
}


