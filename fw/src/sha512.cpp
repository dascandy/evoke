#include "fw/sha512.h"
#include "openssl/sha.h"
#if __linux__
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#else
#include <filesystem>
#include <fstream>
#endif

std::array<uint8_t, 64> sha512(const fs::path& path) 
{
    std::array<uint8_t, 64> hash;
#if __linux__
    int fd = open(path.c_str(), O_RDONLY);
    size_t filesize = fs::file_size(path);
    void* target = mmap(nullptr, filesize, PROT_READ, MAP_SHARED, fd, 0);
    close(fd);
    SHA512((const unsigned char*)target, filesize, hash.data());
    munmap(target, filesize);
#else
    std::ifstream inFile(path);
    auto filesize = fs::file_size(path);
    auto target = std::make_unique<std::uint8_t[]>(filesize);
    inFile.read(reinterpret_cast<char*>(target.get()), filesize);
    SHA512(target.get(), filesize, hash.data());
#endif
    return hash;
}

