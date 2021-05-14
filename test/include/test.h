#pragma once

#include <string>
#include <fw/filesystem.hpp>
#include <cstddef>
#include <catch2/catch.hpp>

struct testenvironment {
  testenvironment() 
  : current(fs::current_path())
  {
    fs::error_code ec;
    fs::remove_all("test_temp", ec);
    fs::create_directory("test_temp");
    fs::current_path("test_temp");
  }
  ~testenvironment() {
    fs::current_path(current);
//    if (!std::unhandled_exception())
      fs::remove_all("test_temp");
  }
  fs::path current;
};

void create(const fs::path& path, const char* data);
bool file_exists(const fs::path& path);
uint64_t hash(const fs::path& path);
std::pair<std::string, int> run(const std::string& cmdline);
std::pair<std::string, int> run_evoke(const std::string& args);


