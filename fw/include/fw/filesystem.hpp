#pragma once

#if 1
#    include <boost/filesystem.hpp>
namespace filesystem = boost::filesystem;
#else
#    include <filesystem>
namespace filesystem = std::experimental::filesystem;
#endif

using error_code = boost::system::error_code;