#include "Project.h"
#include <algorithm>
#include <boost/filesystem.hpp>
#include "Component.h"
#include "Configuration.h"
#include <fcntl.h>
#include "File.h"
#include <sys/mman.h>
#include <unistd.h>
#include "known.h"

Project::Project() {
  projectRoot = boost::filesystem::current_path();
  Reload();
}

void Project::Reload() {
  unknownHeaders.clear();
  components.clear();
  files.clear();
  LoadFileList();

  std::unordered_map<std::string, std::string> includeLookup;
  std::unordered_map<std::string, std::set<std::string>> collisions;
  CreateIncludeLookupTable(includeLookup, collisions);
  std::unordered_map<std::string, std::vector<std::string>> ambiguous;
  MapIncludesToDependencies(includeLookup, ambiguous);
  if (!ambiguous.empty()) {
    fprintf(stderr, "Ambiguous includes found!\n");
    for (auto &i : ambiguous) {
      fprintf(stderr, "Include name %s could point to %zu files -", i.first.c_str(), i.second.size());
      for (auto& s : i.second) {
        fprintf(stderr, " %s", s.c_str());
      }
      fprintf(stderr, "\n");
    }
  }
  PropagateExternalIncludes();
  ExtractPublicDependencies();
}

File* Project::CreateFile(Component& c, boost::filesystem::path p) {
  std::string subpath = p.string();
  if (subpath[0] == '.' && subpath[1] == '/')
    subpath = subpath.substr(2);
  File f(p, c);
  auto f2 = files.emplace(p.string(), std::move(f));
  return &f2.first->second;
}

std::ostream& operator<<(std::ostream& os, const Project& p) {
  for (auto& c : p.components) {
    os << c.second << "\n";
  }
  os << "Pipeline:\n";
  for (auto& c : p.buildPipeline) {
    os << c << "\n";
  }
  return os;
}

void Project::ReadCodeFrom(File& f, const char* buffer, size_t buffersize) {
    if (buffersize == 0) return;
    size_t offset = 0;
    enum State { None, AfterHash, AfterInclude, InsidePointyIncludeBrackets, InsideStraightIncludeBrackets } state = None;
    // Try to terminate reading the file when we've read the last possible preprocessor command
#ifdef LINUX
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
#endif
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

void Project::ReadCode(std::unordered_map<std::string, File>& files, const boost::filesystem::path &path, Component& comp) {
    File& f = files.emplace(path.generic_string().substr(2), File(path.generic_string().substr(2), comp)).first->second;
    comp.files.insert(&f);
    int fd = open(path.c_str(), O_RDONLY);
    size_t fileSize = boost::filesystem::file_size(path);
    void* p = mmap(NULL, fileSize, PROT_READ, MAP_PRIVATE, fd, 0);
    ReadCodeFrom(f, static_cast<const char*>(p), fileSize);
    munmap(p, fileSize);
    close(fd);
}

bool Project::IsItemBlacklisted(const boost::filesystem::path &path) {
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

bool Project::IsCode(const std::string &ext) {
    static const std::unordered_set<std::string> exts = { ".c", ".C", ".cc", ".cpp", ".m", ".mm", ".h", ".H", ".hpp", ".hh" };
    return exts.count(ext) > 0;
}

bool Project::IsCompilationUnit(const std::string& ext) {
    static const std::unordered_set<std::string> exts = { ".c", ".C", ".cc", ".cpp", ".m", ".mm" };
    return exts.count(ext) > 0;
}

void Project::LoadFileList() {
  std::string root = ".";
  components.emplace(root, boost::filesystem::path(root));
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

      Component& comp = components.emplace(parent.c_str(), parent).first->second;
      
      if (boost::filesystem::is_regular_file(it->status()) &&
          IsCode(it->path().extension().generic_string().c_str())) {
          ReadCode(files, it->path(), comp);
      }
  }
}

void Project::MapIncludesToDependencies(std::unordered_map<std::string, std::string> &includeLookup,
                                        std::unordered_map<std::string, std::vector<std::string>> &ambiguous) {
    for (auto &fp : files) {
        for (auto &p : fp.second.rawIncludes) {
            // If this is a non-pointy bracket include, see if there's a local match first. 
            // If so, it always takes precedence, never needs an include path added, and never is ambiguous (at least, for the compiler).
            std::string fullFilePath = (boost::filesystem::path(fp.first).parent_path() / p.first).generic_string();
            if (!p.second && files.count(fullFilePath)) {
                // This file exists as a local include.
                File* dep = &files.find(fullFilePath)->second;
                dep->hasInclude = true;
                fp.second.dependencies.insert(dep);
            } else {
                // We need to use an include path to find this. So let's see where we end up.
                std::string lowercaseInclude;
                std::transform(p.first.begin(), p.first.end(), std::back_inserter(lowercaseInclude), ::tolower);
                const std::string &fullPath = includeLookup[lowercaseInclude];
                if (fullPath == "INVALID") {
                    // We end up in more than one place. That's an ambiguous include then.
                    ambiguous[lowercaseInclude].push_back(fp.first);
                } else if (files.count(fullPath)) {
                    File *dep = &files.find(fullPath)->second;
                    fp.second.dependencies.insert(dep);

                    std::string inclpath = fullPath.substr(0, fullPath.size() - p.first.size() - 1);
                    if (inclpath.size() == dep->component.root.generic_string().size()) {
                        inclpath = ".";
                    } else if (inclpath.size() > dep->component.root.generic_string().size() + 1) {
                        inclpath = inclpath.substr(dep->component.root.generic_string().size() + 1);
                    } else {
                        inclpath = "";
                    }
                    if (!inclpath.empty()) {
                        dep->includePaths.insert(inclpath);
                    }

                    if (&fp.second.component != &dep->component) {
                        fp.second.component.privDeps.insert(&dep->component);
                        dep->hasExternalInclude = true;
                    }
                    dep->hasInclude = true;
                } else if (!IsKnownHeader(p.first)) {
                    unknownHeaders.insert(p.first);
                }
            }
        }
    }
}

void Project::PropagateExternalIncludes() {
    bool foundChange;
    do {
        foundChange = false;
        for (auto &fp : files) {
            if (fp.second.hasExternalInclude) {
                for (auto &dep : fp.second.dependencies) {
                    if (!dep->hasExternalInclude && &dep->component == &fp.second.component) {
                        dep->hasExternalInclude = true;
                        foundChange = true;
                    }
                }
            }
        }
    } while (foundChange);
}

void Project::CreateIncludeLookupTable(std::unordered_map<std::string, std::string> &includeLookup,
                                       std::unordered_map<std::string, std::set<std::string>> &collisions) {
    for (auto &p : files) {
        std::string lowercasePath;
        std::transform(p.first.begin(), p.first.end(), std::back_inserter(lowercasePath), ::tolower);
        const char *pa = lowercasePath.c_str();
        while ((pa = strstr(pa + 1, "/"))) {
            std::string &ref = includeLookup[pa + 1];
            if (ref.size() == 0) {
                ref = p.first;
            } else {
                collisions[pa + 1].insert(p.first);
                if (ref != "INVALID") {
                    collisions[pa + 1].insert(ref);
                }
                ref = "INVALID";
            }
        }
    }
}

void Project::ExtractPublicDependencies() {
    for (auto &c : components) {
        bool hasExtIncludes = false;
        Component &comp = c.second;
        for (auto &fp : comp.files) {
            if (fp->hasExternalInclude) {
                hasExtIncludes = true;
                for (auto &dep : fp->dependencies) {
                    comp.privDeps.erase(&dep->component);
                    comp.pubDeps.insert(&dep->component);
                }
            }
        }
        comp.pubDeps.erase(&comp);
        comp.privDeps.erase(&comp);
        comp.type = hasExtIncludes ? "library" : "executable";
    }
}


