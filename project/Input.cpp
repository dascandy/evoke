#include "Component.h"
#include "File.h"
#include "Configuration.h"
#include <boost/filesystem/fstream.hpp>
#include "Input.h"
#include <algorithm>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

static void ReadCodeFrom(File& f, const char* buffer, size_t buffersize) {
    if (buffersize == 0) return;
    size_t offset = 0;
    enum State { None, AfterHash, AfterInclude, InsidePointyIncludeBrackets, InsideStraightIncludeBrackets } state = None;
    // Try to terminate reading the file when we've read the last possible preprocessor command
    const char* lastHash = static_cast<const char*>(memrchr(buffer, '#', buffersize));
    if (lastHash) {
        // Common case optimization: Header with inclusion guard
        if (strncmp(lastHash, "#endif", 6) == 0) {
            lastHash = static_cast<const char*>(memrchr(buffer, '#', lastHash - buffer - 1));
        }
        if (lastHash) {
            const char* nextNewline = static_cast<const char*>(memchr(lastHash, '\n', buffersize - (lastHash - buffer)));
            if (nextNewline) {
                buffersize = nextNewline - buffer;
            }
        }
    }
    const char* nextHash = static_cast<const char*>(memchr(buffer+offset, '#', buffersize-offset));
    const char* nextSlash = static_cast<const char*>(memchr(buffer+offset, '/', buffersize-offset));
    size_t start = 0;
    while (offset < buffersize) {
        switch (state) {
        case None:
        {
            if (nextHash && nextHash < buffer + offset) nextHash = static_cast<const char*>(memchr(buffer+offset, '#', buffersize-offset));
            if (nextHash == NULL) return;
            if (nextSlash && nextSlash < buffer + offset) nextSlash = static_cast<const char*>(memchr(buffer+offset, '/', buffersize-offset));
            if (nextSlash && nextSlash < nextHash) {
                offset = nextSlash - buffer;
                if (buffer[offset + 1] == '/') {
                    offset = static_cast<const char*>(memchr(buffer+offset, '\n', buffersize-offset)) - buffer;
                }
                else if (buffer[offset + 1] == '*') {
                    do {
                        const char* endSlash = static_cast<const char*>(memchr(buffer + offset + 1, '/', buffersize - offset));
                        if (!endSlash) return;
                        offset = endSlash - buffer;
                    } while (buffer[offset-1] != '*');
                }
            } else {
                offset = nextHash - buffer;
                state = AfterHash;
            }
        }
            break;
        case AfterHash:
            switch (buffer[offset]) {
            case ' ':
            case '\t':
                break;
            case 'i':
                if (buffer[offset + 1] == 'm' &&
                    buffer[offset + 2] == 'p' &&
                    buffer[offset + 3] == 'o' &&
                    buffer[offset + 4] == 'r' &&
                    buffer[offset + 5] == 't') {
                    state = AfterInclude;
                    offset += 5;
                }
                else if (buffer[offset + 1] == 'n' &&
                    buffer[offset + 2] == 'c' &&
                    buffer[offset + 3] == 'l' &&
                    buffer[offset + 4] == 'u' &&
                    buffer[offset + 5] == 'd' &&
                    buffer[offset + 6] == 'e') {
                    state = AfterInclude;
                    offset += 6;
                }
                else
                {
                    state = None;
                }
                break;
            default:
                state = None;
                break;
            }
            break;
        case AfterInclude:
            switch (buffer[offset]) {
            case ' ':
            case '\t':
                break;
            case '<':
                start = offset + 1;
                state = InsidePointyIncludeBrackets;
                break;
            case '"':
                start = offset + 1;
                state = InsideStraightIncludeBrackets;
                break;
            default:
                // Buggy code, skip over this include.
                state = None;
                break;
            }
            break;
        case InsidePointyIncludeBrackets:
            switch (buffer[offset]) {
            case '\n':
                state = None; // Buggy code, skip over this include.
                break;
            case '>':
                f.AddIncludeStmt(true, std::string(&buffer[start], &buffer[offset]));
                state = None;
                break;
            }
            break;
        case InsideStraightIncludeBrackets:
            switch (buffer[offset]) {
            case '\n':
                state = None; // Buggy code, skip over this include.
                break;
            case '\"':
                f.AddIncludeStmt(false, std::string(&buffer[start], &buffer[offset]));
                state = None;
                break;
            }
            break;
        }
        offset++;
    }
}

static void ReadCode(std::unordered_map<std::string, File>& files, const boost::filesystem::path &path, Component& comp) {
    File& f = files.emplace(path.generic_string(), File(path.generic_string().substr(comp.root.size()+1), comp)).first->second;
    comp.files.insert(&f);
    int fd = open(path.c_str(), O_RDONLY);
    size_t fileSize = boost::filesystem::file_size(path);
    void* p = mmap(NULL, fileSize, PROT_READ, MAP_PRIVATE, fd, 0);
    ReadCodeFrom(f, static_cast<const char*>(p), fileSize);
    munmap(p, fileSize);
    close(fd);
}

static bool IsItemBlacklisted(const boost::filesystem::path &path) {
    std::string pathS = path.generic_string();
    std::string fileName = path.filename().generic_string();
    for (auto& s : Configuration::Get().blacklist) {
        if (pathS.compare(2, s.size(), s) == 0) {
            return true;
        }
        if (s == fileName) 
            return true;
    }
    return false;
}

static bool IsCode(const std::string &ext) {
    static const std::unordered_set<std::string> exts = { ".c", ".C", ".cc", ".cpp", ".m", ".mm", ".h", ".H", ".hpp", ".hh" };
    return exts.count(ext) > 0;
}

void LoadFileList(std::unordered_map<std::string, Component> &components,
                  std::unordered_map<std::string, File>& files,
                  const boost::filesystem::path& sourceDir) {
    boost::filesystem::path outputpath = boost::filesystem::current_path();
    boost::filesystem::current_path(sourceDir.c_str());
    AddComponentDefinition(components, ".");
    for (boost::filesystem::recursive_directory_iterator it("."), end;
         it != end; ++it) {
        const auto &parent = it->path().parent_path();
        // skip hidden files and dirs
        const auto& fileName = it->path().filename().generic_string();
        if ((fileName.size() >= 2 && fileName[0] == '.') ||
            IsItemBlacklisted(it->path())) {
            it.disable_recursion_pending();
            continue;
        }       

        Component& comp = AddComponentDefinition(components, parent);
        
        if (boost::filesystem::is_regular_file(it->status()) &&
            IsCode(it->path().extension().generic_string().c_str())) {
            ReadCode(files, it->path(), comp);
        }
    }
    boost::filesystem::current_path(outputpath);
}

void ForgetEmptyComponents(std::unordered_map<std::string, Component> &components) {
  for (auto it = begin(components); it != end(components);) {
    if (it->second.files.empty())
      it = components.erase(it);
    else
      ++it;
  }
}


