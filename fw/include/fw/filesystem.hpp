#pragma once

#if 1
#    include <boost/filesystem.hpp>
namespace filesystem = boost::filesystem;
using error_code = boost::system::error_code;
#else
#    include <filesystem>
namespace filesystem = std::filesystem;
using error_code = std::error_code;
#endif
