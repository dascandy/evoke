#include "fw/sha512.h"
#include "openssl/sha.h"
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <filesystem>

std::array<uint8_t, 64> sha512(const fs::path& path) 
{
    std::array<uint8_t, 64> hash = {};
    size_t fileSize = fs::file_size(path.string());
    if(fileSize > 0)
    {
        using namespace boost::interprocess;
        file_mapping file(path.string().c_str(), read_only);
        mapped_region region(file, read_only);
        SHA512(static_cast<unsigned char *>(region.get_address()), region.get_size(), hash.data());
    }
    return hash;
}

