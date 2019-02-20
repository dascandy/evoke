#include "Component.h"
#include "Configuration.h"
#include "File.h"
#include "PendingCommand.h"
#include "Project.h"
#include "Toolset.h"
#include "boost/algorithm/string/classification.hpp"
#include "dotted.h"
#include "globaloptions.h"

#include <algorithm>
#include <boost/algorithm/string/split.hpp>
#include <stack>
#include <unordered_set>

GccToolset::GccToolset() 
{
  compiler = "g++";
  linker = "g++";
  archiver = "ar";
}

void GccToolset::SetParameter(const std::string& key, const std::string& value) {
  if (key == "compiler") compiler = value;
  else if (key == "linker") linker = value;
  else if (key == "archiver") archiver = value;
  else throw std::runtime_error("Invalid parameter for GCC toolchain: " + key);
}

std::string GccToolset::getObjNameFor(const File& file) {
  return file.path.generic_string() + ".o";
}

std::string GccToolset::getLibNameFor(const Component &component)
{
    return "lib" + getNameFor(component) + ".a";
}

std::string GccToolset::getExeNameFor(const Component &component)
{
    return getNameFor(component);
}

std::string GccToolset::getUnityCommand(const std::string& program, const std::string& compileFlags, const std::string& outputFile, const File* inputFile, const std::set<std::string>& includes, std::vector<std::vector<Component*>> linkDeps) {
  std::string command = program + " -c " + compileFlags + " -o " + outputFile + " " + inputFile->path.generic_string();
  for (auto& i : includes) command += " -I" + i;
  for(auto d : linkDeps)
  {
    if(d.size() == 1)
    {
      command += " -l" + d.front()->root.string();
    }
    else
    {
      command += " -Wl,--start-group";
      for(auto &c : d)
      {
        command += " -l" + c->root.string();
      }
      command += " -Wl,--end-group";
    }
  }
  return command;
}

std::string GccToolset::getCompileCommand(const std::string& program, const std::string& compileFlags, const std::string& outputFile, const File* inputFile, const std::set<std::string>& includes) {
  std::string command = program + " -c " + compileFlags + " -o " + outputFile + " " + inputFile->path.generic_string();
  for (auto& i : includes) command += " -I" + i;
  return command;
}

std::string GccToolset::getArchiverCommand(const std::string& program, const std::string& outputFile, const std::vector<File*> inputs) {
  std::string command = program + " rcs " + outputFile;
  for(auto &file : inputs)
  {
    command += " " + file->path.generic_string();
  }
  return command;
}

std::string GccToolset::getLinkerCommand(const std::string& program, const std::string& outputFile, const std::vector<File*> objects, std::vector<std::vector<Component*>> linkDeps) {
  std::string command = program + " -pthread -o " + outputFile;
  for(auto &file : objects)
  {
      command += " " + file->path.string();
  }
  command += " -Llib";
  for(auto d : linkDeps)
  {
    if(d.size() == 1)
    {
      command += " -l" + d.front()->root.string();
    }
    else
    {
      command += " -Wl,--start-group";
      for(auto &c : d)
      {
        command += " -l" + c->root.string();
      }
      command += " -Wl,--end-group";
    }
  }
  return command;
}

std::string GccToolset::getUnittestCommand(const std::string& program) {
  return "./" + program;
}
/*
void GccToolset::CreateCommandsForUnity(Project &project)
{
    for(auto &p : project.components)
    {
        auto &component = p.second;
        if (component.type == "library") continue;

        std::vector<std::vector<Component *>> allDeps = GetTransitiveAllDeps(component);
        std::vector<Component*> deps;
        std::vector<File*> files;
        std::string linkline;
        std::unordered_set<std::string> includes;
        filesystem::create_directories("unity");
        filesystem::path outputFile = std::string("unity") + "/" + getExeNameFor(component) + ".cpp";
        File* of = project.CreateFile(component, outputFile);
        std::ofstream out(outputFile.generic_string());
        for (auto& v : allDeps) for (auto& c : v) {
            if (c->isBinary) {
                linkline += " -l" + getNameFor(*c);
            } else {
                for(auto &d : getIncludePathsFor(component))
                {
                    includes.insert(d);
                }
                for (auto& f : c->files) {
                    files.push_back(f);
                    if(File::isTranslationUnit(f->path))
                    {
                        out << "#include \"../" + f->path.generic_string() << "\"\n";
                    }
                }
            }
        }

        filesystem::path exeFile = "bin/" + getExeNameFor(component);
        std::string includeString;
        for (auto& i : includes) {
            includeString += " -I" + i;
        }
        std::shared_ptr<PendingCommand> pc = std::make_shared<PendingCommand>(compiler + " " + Configuration::Get().compileFlags + " " + includeString + " -pthread -o " + exeFile.generic_string() + " " + outputFile.generic_string() + linkline);

        File *executable = project.CreateFile(component, exeFile);
        pc->AddOutput(executable);
        pc->AddInput(of);
        for (auto& f : files)
            pc->AddInput(f);
        pc->Check();
        component.commands.push_back(pc);
        if(component.type == "unittest")
        {
            pc = std::make_shared<PendingCommand>(exeFile.string());
            exeFile += ".log";
            pc->AddInput(executable);
            pc->AddOutput(project.CreateFile(component, exeFile.string()));
            pc->Check();
            component.commands.push_back(pc);
        }
    }
}
*/
GlobalOptions GccToolset::ParseGeneralOptions(const std::string &options)
{
    GlobalOptions opts;
    //std::vector<std::string> parts = splitWithQuotes(options);
    std::vector<std::string> parts;
    boost::split(parts, options, boost::algorithm::is_any_of(" \t"), boost::algorithm::token_compress_on);

    for(const auto &opt : parts)
    {
        if(opt.find("-I") == 0)
        {
            opts.include.emplace_back(opt.substr(2));
        }
        else if(opt.find("-L") == 0)
        {
            opts.link.emplace_back(opt);
        }
        else if(opt == "-pthread")
        {
            opts.compile.emplace_back("-pthread");
            opts.link.emplace_back("-lpthread");
        }
        else
        {
            opts.compile.emplace_back(opt);
        }
    }
    return opts;
}
