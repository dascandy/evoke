#pragma once

#include <string>
#include <boost/filesystem.hpp>
#include <catch.hpp>

struct testenvironment {
  testenvironment() 
  : current(boost::filesystem::current_path())
  {
    boost::system::error_code ec;
    boost::filesystem::remove_all("test_temp", ec);
    boost::filesystem::create_directory("test_temp");
    boost::filesystem::current_path("test_temp");
  }
  ~testenvironment() {
    boost::filesystem::current_path(current);
//    if (!std::unhandled_exception())
      boost::filesystem::remove_all("test_temp");
  }
  boost::filesystem::path current;
};

void create(const boost::filesystem::path& path, const char* data);
bool file_exists(const boost::filesystem::path& path);
uint64_t hash(const boost::filesystem::path& path);
std::pair<std::string, int> run(const std::string& cmdline);


