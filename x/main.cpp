#include "Project.h"
#include <iostream>
#include "Toolset.h"
#include "view/values.h"

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
    UbuntuToolset ut;
    for (auto& c : values(op.components)) {
      ut.CreateCommandsFor(op, c);
    }
    std::vector<PendingCommand*> commands;
    for (auto& c : op.components) {
      commands.insert(commands.end(), c.second.commands.begin(), c.second.commands.end());
    }

    std::cout << op;
    std::cout << commands;
    return 0;
}


