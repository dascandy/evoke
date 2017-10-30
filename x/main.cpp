#include "Project.h"
#include <iostream>
#include "Toolset.h"
#include "view/values.h"

int main(int, const char **) {
    Project op;
    UbuntuToolset ut;
    for (auto& c : values(op.components)) {
      ut.CreateCommandsFor(op, c);
    }

    std::cout << op;
    return 0;
}


