#define CATCH_CONFIG_MAIN
#include "test.h"
#include <fstream>
#include <boost/process.hpp>

bool file_exists(const fs::path& path) {
  boost::system::error_code ec;
  return fs::is_regular_file(path, ec);
}

void create(const fs::path& path, const char* data) {
  fs::create_directories(path.parent_path());
  fs::ofstream(path) << data;
}

uint64_t hash(const fs::path& path) {
  fs::ifstream is(path);
  std::string data;
  data.resize(fs::file_size(path));
  is.read(reinterpret_cast<char*>(const_cast<char*>(data.data())), data.size());
  return std::hash<std::string>()(data);
}

std::pair<std::string, int> run(const std::string& cmdline) {
    boost::process::ipstream pipe_stream;
    boost::process::child child(cmdline, (boost::process::std_out & boost::process::std_err) > pipe_stream);
    std::string outbuffer;
    std::string line;
    while(std::getline(pipe_stream, line))
        outbuffer += line + "\n";

    child.wait();
    int errorcode = child.exit_code();
    return { outbuffer, errorcode };
}

std::pair<std::string, int> run_evoke(const std::string& args) {
    return run("../build/gcc/bin/evoke " + args);
}


