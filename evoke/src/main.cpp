#include "Executor.h"
#include "Project.h"
#include "Reporter.h"
#include "Toolset.h"
#include "JsonCompileDb.h"
#include "CMakeListsTxt.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <thread>
#include "fw/FsWatcher.hpp"
#include "Finder.h"

using namespace std::literals::chrono_literals;

template<typename T>
std::ostream &operator<<(std::ostream &os, std::vector<T> v)
{
    os << "[\n";
    bool first = true;
    for(auto &e : v)
    {
        if(first)
            first = false;
        else
            os << ", ";
        os << *e << "\n";
    }
    os << "]\n";
    return os;
}

void parseArgs(std::vector<std::string> args, std::map<std::string, std::string &> argmap, std::map<std::string, bool &> toggles, std::function<void(std::string)> onUnknown)
{
    for(size_t index = 0; index < args.size();)
    {
        auto toggle_it = toggles.find(args[index]);
        if(toggle_it != toggles.end())
        {
            toggle_it->second = true;
            ++index;
        }
        else
        {
            auto it = argmap.find(args[index]);
            if(it != argmap.end() && index + 1 != args.size())
            {
                it->second = args[index + 1];
                index += 2;
            }
            else
            {
                onUnknown(args[index]);
                ++index;
            }
        }
    }
}

static std::string default_toolset() {
#ifdef _WIN32
  return "windows";
#elif defined(APPLE)
  return "apple";
#else
  return "linux";
#endif
}

uint64_t parseMemoryLimit(std::string str) {
  if (str == "none") return std::numeric_limits<uint64_t>::max();
  if (str == "available") return 0;
  uint64_t limit = 1;
  switch(str.back()) {
    case 'k': case 'K': limit = 1024; break;
    case 'm': case 'M': limit = 1024 * 1024; break;
    case 'g': case 'G': limit = 1024 * 1024 * 1024; break;
    case 't': case 'T': limit = 1024 * 1024 * 1024 * 1024ULL; break;
  }
  if (limit != 1) str.pop_back();
  limit *= std::stoul(str);
  return limit;
}

int main(int argc, const char **argv)
{
    std::string toolsetname = default_toolset();
    std::string rootpath = fs::current_path().generic_string();
    std::string jobcount = std::to_string(std::max(4u, std::thread::hardware_concurrency()));
    std::string reporterName = "guess";
    std::string memoryLimit = "available";
    bool compilation_database = false;
    bool cmakelists = false;
    bool verbose = false;
    bool unitybuild = false;
    bool daemon = false;
    std::map<std::string, std::vector<std::string>> targetsToBuild;
    parseArgs(std::vector<std::string>(argv + 1, argv + argc), {{"--root", rootpath}, {"-j", jobcount}, {"-r", reporterName}, {"-m", memoryLimit}}, {{"-cp", compilation_database}, {"-v", verbose}, {"-cm", cmakelists}, {"-u", unitybuild}, {"-d", daemon}},
        [&](std::string arg) {
        // This feels really icky, but it's the way to make this work. TODO: extract this into a class.
        static bool insideTarget = false;
        if (arg.empty()) {
            std::cout << "Invalid argument: " << arg << "\n";
        } else if (insideTarget) {
            if (toolsetname.empty()) {
                toolsetname = arg;
                targetsToBuild[toolsetname];
                insideTarget = false;
            } else {
                targetsToBuild[toolsetname].push_back(arg);
            }
        } else if (arg == "-t") {
            insideTarget = true;
            toolsetname.clear();
        } else if (arg.empty() || arg[0] == '-') {
            std::cout << "Invalid argument: " << arg << "\n";
        } else {
            targetsToBuild[toolsetname].push_back(arg);
        }
    });
    if (targetsToBuild.empty()) targetsToBuild[toolsetname];
    if (daemon && reporterName == "guess") {
        reporterName = "daemon";
    }
    std::unique_ptr<Reporter> reporter = Reporter::Get(reporterName);
    Executor ex(std::stoul(jobcount), parseMemoryLimit(memoryLimit), *reporter);
    Project op(rootpath);
    if(!op.unknownHeaders.empty())
    {
        // Report missing headers as error. Build script should handle this gracefully and reinvoke Evoke after fetching the missing headers.
    }
    for(auto &u : op.unknownHeaders)
    {
        std::cerr << "Unknown header: " << u << "\n";
    }
    auto GenerateCommands = [&]() {
        for (auto& p : targetsToBuild) {
            std::unique_ptr<Toolset> toolset = GetToolsetByName(p.first);
            if(unitybuild)
            {
                toolset->CreateCommandsForUnity(op);
            }
            else
            {
                toolset->CreateCommandsFor(op);
            }
        }
        // TODO: filter this on actually useful commands
        for(auto &comp : op.components)
        {
            for(auto &c : comp.second.commands)
            {
                c->Check();
            }
        }
        // TODO: filter this on actually useful commands
        for(auto &comp : op.components)
        {
            for(auto &c : comp.second.commands)
            {
                ex.Run(c);
            }
        }
    };
    auto UpdateAndRunJobs = [&](fs::path changedFile, Change change){
      while (1) {
        try {
          std::lock_guard<std::mutex> l(ex.m);
          bool reloaded = op.FileUpdate(changedFile, change);
          bool isPackageOrToolsetChange = changedFile.extension() == ".toolset" ||
                                          changedFile.filename() == "packages.conf";
          if (isPackageOrToolsetChange) {
              op.Reload();
              reloaded = true;
          }
          if (reloaded) {
              GenerateCommands();
          }
          if (compilation_database)
          {
              std::ofstream os("compile_commands.json");
              dumpJsonCompileDb(os, op);
          }
          ex.RunMoreCommands();
          return;
        } catch (...) {
          // Any exception is from more filesystem changes while the iterator was moving. Just retry.
        }
      }
    };
    if (daemon) {
        reporterName = "daemon";
        FsWatch(rootpath, UpdateAndRunJobs);
    }
    // If this is in daemon mode, the future returns when the user sends a SIGTERM, SIGINT or such. 
    // If not, it blocks until there are no jobs left to run.
    GenerateCommands();
    std::future<void> finished;
    {
        std::lock_guard<std::mutex> l(ex.m);
        finished = ex.Mode(daemon);
        ex.RunMoreCommands();
    }
    finished.get();
    if(cmakelists)
    {
        std::unique_ptr<Toolset> toolset = GetToolsetByName(toolsetname);
        CMakeProjectExporter exporter{op};
        exporter.createCMakeListsFiles(*toolset);
    }
    if(verbose)
    {
        std::cout << op;
    }
    printf("\n\n");
    return ex.AllSuccess() ? EXIT_SUCCESS : EXIT_FAILURE;
}


