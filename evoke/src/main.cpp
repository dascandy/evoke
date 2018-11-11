#include "Project.h"
#include <iostream>
#include "Toolset.h"
#include "Executor.h"
#include <thread>
#include <chrono>
#include <map>
using namespace std::literals::chrono_literals;

template <typename T>
std::ostream& operator<<(std::ostream& os, std::vector<T> v) {
  os << "[\n";
  bool first = true;
  for (auto& e : v) {
    if (first) first = false; else os << ", ";
    os << *e << "\n";
  }
  os << "]\n";
  return os;
}

void parseArgs(std::vector<std::string> args, std::map<std::string, std::string&> argmap) {
  if (args.size() % 1)
    std::cout << "Lone argument at end? " << args.back() << "\n";
  for (size_t index = 0; index + 1 < args.size(); index += 2) {
    auto it = argmap.find(args[index]);
    if (it != argmap.end()) {
      it->second = args[index+1];
    } else {
      std::cout << "Invalid argument: " << args[index] << "\n";
    }
  }
}

int main(int argc, const char **argv) {
  std::string toolsetname = "ubuntu";
  std::string compdbname = "";
  std::string verbose = "";
  parseArgs(std::vector<std::string>(argv+1, argv + argc), { { "-t", toolsetname }, { "-cp", compdbname }, { "-v", verbose } });
  Project op;
  if (!op.unknownHeaders.empty()) {
    /*
      // TODO: allow building without package fetching somehow
      std::string fetch = "accio fetch";
      std::vector<std::string> hdrsToFetch(op.unknownHeaders.begin(), op.unknownHeaders.end());
      for (auto& hdr : hdrsToFetch) fetch += " " + hdr;
      system(fetch.c_str());
      op.Reload();
    */
  }
  for (auto& u : op.unknownHeaders) {
      std::cerr << "Unknown header: " << u << "\n";
  }
  std::unique_ptr<Toolset> toolset = GetToolsetByName(toolsetname);
  toolset->CreateCommandsFor(op);
  if (!compdbname.empty()) {
    std::ofstream os(compdbname);
    op.dumpJsonCompileDb(os);
  }
  if (!verbose.empty()) {
    std::cout << op;
  }
  Executor ex;
  for (auto& comp : op.components) {
    for (auto& c : comp.second.commands) {
      if (c->state == PendingCommand::ToBeRun) 
        ex.Run(c);
    }
  }
  ex.Start();
  while (ex.Busy()) { std::this_thread::sleep_for(1s); }
  printf("\n\n");
  return 0;
}


