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
        if (c->state == PendingCommand::ToBeRun) 
          ex.Run(c);
      }
    }
//    std::cerr << op;
    ex.Start();
    while (ex.Busy()) { std::this_thread::sleep_for(1s); }
    printf("\n\n");
    return 0;
}


