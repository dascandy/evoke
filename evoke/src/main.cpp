#include "Project.h"
#include <iostream>
#include "Toolset.h"
#include "values.h"
#include "Executor.h"
#include <thread>
#include <chrono>
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

int main(int, const char **) {
    Project op;
    if (!op.unknownHeaders.empty()) {
        // TODO: allow building without package fetching somehow
        std::string fetch = "accio fetch";
        std::vector<std::string> hdrsToFetch(op.unknownHeaders.begin(), op.unknownHeaders.end());
        for (auto& hdr : hdrsToFetch) fetch += " " + hdr;
        system(fetch.c_str());
        op.Reload();
    }
    for (auto& u : op.unknownHeaders) {
        std::cerr << "Unknown header: " << u << "\n";
    }

    UbuntuToolset ut;
    for (auto& c : values(op.components)) {
      ut.CreateCommandsFor(op, c);
    }
    Executor ex;
    for (auto& comp : op.components) {
      for (auto& c : comp.second.commands) {
        printf("%s %d\n", c->commandToRun.c_str(), c->state);
        if (c->state == PendingCommand::ToBeRun) 
          ex.Run(c);
      }
    }
    std::cout << op;
    ex.Start();
    while (ex.Busy()) { std::this_thread::sleep_for(1s); }
    printf("\n\n");
    return 0;
}


