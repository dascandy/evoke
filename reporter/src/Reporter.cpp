#include "Reporter.h"
#include <string>
#include <iostream>
#ifndef _WIN32
#include <unistd.h>
#endif

std::unique_ptr<Reporter> Reporter::Get(const std::string& name) {
  if (name == "simple") {
    return std::make_unique<SimpleReporter>();
  } else if (name == "console") {
    return std::make_unique<ConsoleReporter>();
  } else if (name == "guess") {
#ifndef _WIN32
    if (isatty(0) && isatty(1)) {
      return std::make_unique<ConsoleReporter>();
    } else {
#endif
      return std::make_unique<SimpleReporter>();
#ifndef _WIN32
    }
#endif
  } else {
    std::cout << "Unknown reporter: " << name << "\n";
    return std::make_unique<SimpleReporter>();
  }
}


