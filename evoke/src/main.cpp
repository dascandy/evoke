#include "Configuration.h"
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

void parseArgs(std::vector<std::string> args, std::map<std::string, std::string &> argmap, std::map<std::string, bool &> toggles)
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
                std::cout << "Invalid argument: " << args[index] << "\n";
                ++index;
            }
        }
    }
}

int main(int argc, const char **argv)
{
    std::string toolsetname = Configuration::Get().toolchain;

    std::cout << "Building for " << toolsetname << std::endl;

    std::string rootpath = filesystem::current_path().generic_string();
    std::string jobcount = std::to_string(std::max(4u, std::thread::hardware_concurrency()));
    std::string reporterName = "guess";
    bool compilation_database = false;
    bool cmakelists = false;
    bool verbose = false;
    bool unitybuild = false;
    parseArgs(std::vector<std::string>(argv + 1, argv + argc), {{"-t", toolsetname}, {"--root", rootpath}, {"-j", jobcount}, {"-r", reporterName}}, {{"-cp", compilation_database}, {"-v", verbose}, {"-cm", cmakelists}, {"-u", unitybuild}});
    Project op(rootpath);
    if(!op.unknownHeaders.empty())
    {
        // Report missing headers as error. Build script should handle this gracefully and reinvoke Evoke after fetching the missing headers.
        /*
      // TODO: allow building without package fetching somehow
      std::string fetch = "accio fetch";
      std::vector<std::string> hdrsToFetch(op.unknownHeaders.begin(), op.unknownHeaders.end());
      for (auto& hdr : hdrsToFetch) fetch += " " + hdr;
      system(fetch.c_str());
      op.Reload();
    */
    }
    for(auto &u : op.unknownHeaders)
    {
        std::cerr << "Unknown header: " << u << "\n";
    }
    std::unique_ptr<Toolset> toolset = GetToolsetByName(toolsetname);
    if(unitybuild)
    {
        toolset->CreateCommandsForUnity(op);
    }
    else
    {
        toolset->CreateCommandsFor(op);
    }
    if(compilation_database)
    {
        std::ofstream os("compile_commands.json");
        dumpJsonCompileDb(os, op);
    }
    if(cmakelists)
    {
        auto opts = toolset->ParseGeneralOptions(Configuration::Get().compileFlags);
        CMakeProjectExporter exporter{op};
        exporter.createCMakeListsFiles(opts);
    }
    if(verbose)
    {
        std::cout << op;
    }
    std::unique_ptr<Reporter> reporter = Reporter::Get(reporterName);
    Executor ex(std::stoul(jobcount), *reporter);
    for(auto &comp : op.components)
    {
        for(auto &c : comp.second.commands)
        {
            if(c->state == PendingCommand::ToBeRun)
                ex.Run(c);
        }
    }
    ex.Start().get();
    printf("\n\n");
}
