#pragma once

#if defined(_MSC_VER) || (__GNUC__ < 8)

#    include <boost/filesystem.hpp>

namespace filesystem
{
using namespace boost::filesystem;
using boost::system::error_code;
}

#else

#    include <filesystem>
#    include <fstream>

namespace filesystem
{
using namespace std::filesystem;
using std::error_code;
using std::ofstream;
}

#endif
