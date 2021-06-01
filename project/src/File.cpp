#include "File.h"
#include "PendingCommand.h"
#include "fw/sha512.h"

File::File(const fs::path &path, Component &component) :
    path(path),
    component(component),
    hasExternalInclude(false),
    hasInclude(false),
    hash()
{
    try {
        hash = sha512(path);
    } catch (...) {
    }
}

void File::reloadHash() {
    auto newHash = sha512(path);
    hash = newHash;
}

bool File::isHeader(const fs::path &path)
{
    static const std::unordered_set<std::string> exts = {".h", ".H", ".hpp", ".hh", ".tcc", ".ipp", ".inc"};
    return exts.find(path.extension().generic_string()) != exts.end();
}

bool File::isTranslationUnit(const fs::path &path)
{
    static const std::unordered_set<std::string> exts = {".c", ".C", ".cc", ".cpp", ".cppm", ".m", ".mm", ".s", ".S"};
    return exts.find(path.extension().generic_string()) != exts.end();
}

bool File::isTranslationUnit() const
{
    return isTranslationUnit(path);
}

bool File::isHeader() const
{
    return isHeader(path);
}

bool File::Exists() const {
  try {
    return fs::is_regular_file(path);
  } catch (...) {
    return false;
  }
}

void File::FileUpdated() {
    // No state change, oddly enough
    // - If it was source it's still source
    // - If it is not source, we'll have to assume it was our command changing it, and that command will set the appropriate new state
    // - If the user modified build intermediates, they get what they deserve
    auto newHash = sha512(path);
    if (newHash != hash) {
        for (auto listener : listeners) {
            listener->Check();
        }
    }
}

