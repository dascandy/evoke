#include "Component.h"
#include "Configuration.h"
#include "File.h"
#include "PendingCommand.h"
#include "Project.h"
#include "Toolset.h"
#include "dotted.h"

#include <algorithm>
#include <stack>

//https://blogs.msdn.microsoft.com/vcblog/2015/12/03/c-modules-in-vs-2015-update-1/
// Enable modules support for MSVC
//"/experimental:module /module:stdIfcDir \"$(VC_IFCPath)\" /module:search obj/modules/"

MsvcToolset::MsvcToolset() 
{
  compiler = "cl.exe";
  linker = "link.exe";
  archiver = "lib.exe";
}

void MsvcToolset::SetParameter(const std::string& key, const std::string& value) {
  if (key == "compiler") compiler = value;
  else if (key == "linker") linker = value;
  else if (key == "archiver") archiver = value;
  else throw std::runtime_error("Invalid parameter for MSVC toolchain: " + key);
}

std::string MsvcToolset::getObjNameFor(const File& file) {
  return file.path.generic_string() + ".obj";
}

std::string MsvcToolset::getLibNameFor(const Component &component)
{
    return "lib" + getNameFor(component) + ".lib";
}

std::string MsvcToolset::getExeNameFor(const Component &component)
{
    return getNameFor(component) + ".exe";
}

std::string MsvcToolset::getUnityCommand(const std::string& program, const std::string& compileFlags, const std::string& outputFile, const File* inputFile, const std::set<std::string>& includes, std::vector<std::vector<Component*>> linkDeps) {
  std::string command = program + " /c /EHsc " + compileFlags + " /Fo" + outputFile + " " + inputFile->path.generic_string();
  for (auto& i : includes) command += " /I" + i;
  for(auto& d : linkDeps) for (auto& c : d) {
      command += " -l" + c->root.string();
  }
  return command;
}

std::string MsvcToolset::getCompileCommand(const std::string& program, const std::string& compileFlags, const std::string& outputFile, const File* inputFile, const std::set<std::string>& includes) {
  std::string command = program + " /c /EHsc " + compileFlags + " /Fo" + outputFile + " " + inputFile->path.generic_string();
  for (auto& i : includes) command += " /I" + i;
  return command;
}

std::string MsvcToolset::getArchiverCommand(const std::string& program, const std::string& outputFile, const std::vector<File*> inputs) {
  std::terminate(); // TODO: not implemented.
  std::string command = program + " " + outputFile;
  for(auto &file : inputs)
  {
    command += " " + file->path.generic_string();
  }
  return command;
}

std::string MsvcToolset::getLinkerCommand(const std::string& program, const std::string& outputFile, const std::vector<File*> objects, std::vector<std::vector<Component*>> linkDeps) {
  std::string command = program + " /OUT:" + outputFile;
  for(auto &file : objects)
  {
      command += " " + file->path.string();
  }
  command += " /LIBPATH:lib";
  for(auto& d : linkDeps) for (auto& c : d) {
      command += " -l" + c->root.string();
  }
  return command;
}

std::string MsvcToolset::getUnittestCommand(const std::string& program) {
  return "./" + program;
}
/*
                outputFile = "bin\\" + getExeNameFor(component);
                command = linker + " /OUT:" + outputFile.string();

                for(auto &file : objects)
                {
                    command += " " + file->path.string();
                }
                command += " /LIBPATH:lib";
                std::vector<std::vector<Component *>> linkDeps = GetTransitiveAllDeps(component);
                std::reverse(linkDeps.begin(), linkDeps.end());
                for(auto d : linkDeps)
                {
                    for(auto &c : d)
                    {
                        if(c != &component && !c->isHeaderOnly())
                        {
                            command += " " + c->root.string();
                        }
                    }
                }
                pc = std::make_shared<PendingCommand>(command);
                for(auto &d : linkDeps)
                {
                    for(auto &c : d)
                    {
                        if(c != &component && !c->isHeaderOnly())
                        {
                            pc->AddInput(project.CreateFile(*c, "lib\\" + getLibNameFor(*c)));
                        }
                    }
                }
            }
            File *libraryFile = project.CreateFile(component, outputFile);
            pc->AddOutput(libraryFile);
            for(auto &file : objects)
            {
                pc->AddInput(file);
            }
            pc->Check();
            component.commands.push_back(pc);
            if(component.type == "unittest")
            {
                command = outputFile.string();
                pc = std::make_shared<PendingCommand>(command);
                outputFile += ".log";
                pc->AddInput(libraryFile);
                pc->AddOutput(project.CreateFile(component, outputFile.string()));
                pc->Check();
                component.commands.push_back(pc);
            }
        }
    }
}
*/
