#include "fw/sha512.h"
#include "openssl/sha.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

std::array<uint8_t, 64> sha512(const fs::path& path) {
    int fd = open(path.c_str(), O_RDONLY);
    size_t filesize = fs::file_size(path);
    void* target = mmap(nullptr, filesize, PROT_READ, MAP_SHARED, fd, 0);
    close(fd);
    std::array<uint8_t, 64> hash;
    SHA512((const unsigned char*)target, filesize, hash.data());
    munmap(target, filesize);
    return hash;
}

