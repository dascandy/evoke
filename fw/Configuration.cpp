#include "Configuration.h"
#include <boost/filesystem/fstream.hpp>
#include <iostream>
//#include <stdlib.h>

static std::vector<std::string> split(const std::string& str) {  
  std::vector<std::string> rv;
  const char* s = &str[0], *e = &str[str.size()-1];
  const char* p = s;
  bool inQuotes = false;
  while (s < e) {
    if (*s == ' ' && !inQuotes) {
      if (*p != '"')
        rv.push_back(std::string(p, s));
      else
        rv.push_back(std::string(p+1, s-1));

      p = s+1;
    } else if (*s == '\"') {
      inQuotes = !inQuotes;
    }
    s++;
  }
  if (p != s) rv.push_back(std::string(p, s));
  return rv;
}

Configuration::Configuration()
{
  boost::filesystem::ifstream in("x.conf");
  std::string line;
  while (in.good()) {
    std::getline(in, line);
    while (in.good() && line.back() == '\\') {
      std::string nextLine;
      std::getline(in, nextLine);
      line += nextLine;
    }

    size_t pos = line.find_first_of("#");
    if (pos != std::string::npos)
      line.resize(line.find_first_of("#"));

    pos = line.find(": ");
    if (pos == std::string::npos)
      continue;

    std::string name = line.substr(0, pos);
    std::string value = line.substr(pos+2);
    if (name == "blacklist") { blacklist = split(value); }
    else {
      std::cout << "Ignoring unknown tag in configuration file: " << name << "\n";
    }
  }
}

Configuration& Configuration::Get()
{
  static Configuration config;
  return config;
}


