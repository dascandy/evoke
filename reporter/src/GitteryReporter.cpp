#include "PendingCommand.h"
#include "Reporter.h"

#include <cstdio>
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>

GitteryReporter::GitteryReporter() {
  std::cerr << "[ { \"version\": \"1.0\" }";
}

GitteryReporter::~GitteryReporter() {
  std::cerr << " ]";
}

std::string ConvertSarif(nlohmann::json& s) {
  size_t pathLength = 8 + std::filesystem::current_path().string().size();
  nlohmann::json errors;
  size_t id = 0;
  for (auto& e : s["runs"][0]["results"]) {
    for (auto& location : e["locations"]) {
      std::string filename = std::string(location["physicalLocation"]["artifactLocation"]["uri"]).substr(pathLength);
      size_t line = location["physicalLocation"]["region"]["startLine"];
      size_t column = location["physicalLocation"]["region"]["startColumn"];
      size_t this_id = ++id;
      nlohmann::json err;
      err["id"] = this_id;
      err["location"] = filename + ":" + std::to_string(line) + ":" + std::to_string(column);
      err["message"] = e["message"]["text"];
      err["type"] = e["level"];
      errors.push_back(err);
    }
  }
  if (errors.empty()) return "";
  return errors.dump();
}

static const std::string dashes = "--------------------------------------------\n";
static const std::string dots   = "............................................\n";
static const std::string equals = "============================================\n";
static const std::string failed = ": FAILED:\n";

std::string ConvertCatch(std::string s) {
  size_t dash = s.find(dashes),
         dot = s.find(dots),
         eq = s.find(equals);
  if (eq == std::string::npos) throw std::runtime_error("Not catch format");
  if (dash == std::string::npos && dot == std::string::npos) return "";
  if (dash == std::string::npos || dot == std::string::npos) throw std::runtime_error("Not catch format");
  nlohmann::json errors;
  size_t id = 0;
  while (dash != std::string::npos) {
    size_t nameStart = dash + dashes.size();
    size_t nameEnd = s.find("\n", nameStart);
    std::string name = s.substr(nameStart, nameEnd - nameStart);

    dash = s.find(dashes, nameEnd);
    size_t testLocationStart = dash + dashes.size();
    size_t testLocationEnd = s.find("\n", testLocationStart);
    std::string testLocation = s.substr(testLocationStart, testLocationEnd - testLocationStart);
    size_t this_id = ++id;
    {
      nlohmann::json err;
      err["id"] = this_id;
      err["location"] = testLocation;
      err["message"] = "Test case " + name + " failed";
      err["type"] = "test_fail";
      errors.push_back(err);
    }

    size_t errMsgStart = dot + dots.size();
    while (s[errMsgStart] == '\n') errMsgStart++;
    dash = s.find(dashes, dot);
    dot = s.find(dots, dash);
    size_t errMsgEnd = std::min(dash, eq);
    while (s[errMsgEnd] != '\n') errMsgEnd--;

    std::string errMsg = s.substr(errMsgStart, errMsgEnd - errMsgStart);
    size_t failP = errMsg.find(failed);
    if (failP == std::string::npos) continue;
    size_t nextFail = errMsg.find(failed, failP+1);
    while (nextFail != std::string::npos) {
      size_t end = nextFail;
      while (errMsg[end] != '\n') --end;
      std::string this_error = errMsg.substr(failP + failed.size(), end - failP - failed.size());
      std::string this_location = errMsg.substr(0, failP);
      nlohmann::json err;
      err["id"] = ++id;
      err["parent_id"] = this_id;
      err["location"] = this_location;
      err["message"] = this_error;
      err["type"] = "assertion_fail";
      errors.push_back(err);
      
      errMsg = errMsg.substr(end+1);
      failP = errMsg.find(failed);
      nextFail = errMsg.find(failed, failP+1);
    }

    std::string this_error = errMsg.substr(failP + failed.size());
    std::string this_location = errMsg.substr(0, failP);
    {
      nlohmann::json err;
      err["id"] = ++id;
      err["parent_id"] = this_id;
      err["location"] = this_location;
      err["message"] = this_error;
      err["type"] = "assertion_fail";
      errors.push_back(err);
    }
  }
  return errors.dump();
}

std::string ConvertGcc(std::string s) {
  throw std::runtime_error("Not implemented");
}

std::string ConvertLogging(std::string log) {
  if (log.empty()) return "";

  // SARIF errors
  try {
    size_t clanghack = log.find(" error generated");
    if (clanghack != std::string::npos) {
      while (log[clanghack] != '\n') clanghack--;
      log.resize(clanghack);
    }
    nlohmann::json j = nlohmann::json::parse(log);
    if (j["$schema"] == "https://docs.oasis-open.org/sarif/sarif/v2.1.0/cos02/schemas/sarif-schema-2.1.0.json") 
      return ConvertSarif(j);
  } catch (...) {}
  
  // Catch-style errors
  try {
    if (log.find(equals) != std::string::npos) {
      return ConvertCatch(log);
    }
  } catch (...) {}
  
  // GCC-style errors
  try {
    return ConvertGcc(log);
  } catch (...) {}

  // otherwise, we can't extract useful feedback
  return "";
}

void GitteryReporter::ReportCommand(size_t , std::shared_ptr<PendingCommand> cmd)
{
  std::string mangled = ConvertLogging(cmd->result->output);
  if (mangled.empty()) return;
  std::cerr << ", { \"command\": \"" << cmd->commandToRun << "\", \"output\": ";
  std::cerr << mangled;
  std::cerr << " }";
}


