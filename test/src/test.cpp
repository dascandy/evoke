#define CATCH_CONFIG_MAIN
#include "test.h"
#include <fstream>
#include <boost/process.hpp>

bool file_exists(const boost::filesystem::path& path) {
  boost::system::error_code ec;
  return boost::filesystem::is_regular_file(path, ec);
}

void create(const boost::filesystem::path& path, const char* data) {
  boost::filesystem::create_directories(path.parent_path());
  boost::filesystem::ofstream(path) << data;
}

uint64_t hash(const boost::filesystem::path& path) {
  boost::filesystem::ifstream is(path);
  std::string data;
  data.resize(boost::filesystem::file_size(path));
  is.read(data.data(), data.size());
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


