#include "File.h"

bool File::isHeader(const filesystem::path &path)
{
    static const std::unordered_set<std::string> exts = {".h", ".H", ".hpp", ".hh", ".tcc", ".ipp", ".inc"};
    return exts.find(path.extension().generic_string()) != exts.end();
}

bool File::isTranslationUnit(const filesystem::path &path)
{
    static const std::unordered_set<std::string> exts = {".c", ".C", ".cc", ".cpp", ".cppm", ".m", ".mm"};
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
