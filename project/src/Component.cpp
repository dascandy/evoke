#include "Component.h"

#include "File.h"
#include "PendingCommand.h"
#include "dotted.h"

#include <iterator>

static std::string toName(const filesystem::path &path)
{
    using namespace std;

    auto start = find_if_not(begin(path), end(path), [](auto &part) {
        return part.filename_is_dot();
    });

    if(start == end(path))
    {
        return filesystem::canonical(path).filename().string();
    }

    string out = start->string();

    while(++start != end(path))
    {
        if(!start->filename_is_dot())
        {
            out.append("_").append(start->string());
        }
    }

    return out;
}

Component::Component(const filesystem::path &path, bool isBinary) :
    root(removeDot(path)),
    type("library"),
    isBinary(isBinary),
    name(toName(path))
{
}

bool Component::isHeaderOnly() const
{
    if(isBinary)
        return false;
    for(auto &d : files)
    {
        if(d->isTranslationUnit())
            return false;
    }
    return true;
}

std::string Component::GetName() const
{
    return name;
}

std::ostream &operator<<(std::ostream &os, const Component &component)
{
    if(component.files.empty())
        return os;
    os << "Component " << component.GetName() << " built as " << component.type;
    if(!component.pubDeps.empty())
    {
        os << "\n  Exposes:";
        for(auto &d : component.pubDeps)
        {
            os << " " << d->GetName();
        }
    }
    if(!component.privDeps.empty())
    {
        os << "\n  Uses:";
        for(auto &d : component.privDeps)
        {
            os << " " << d->GetName();
        }
    }
    bool anyExternal = false, anyHeader = false, anySource = false;
    for(auto &d : component.files)
    {
        if(d->hasExternalInclude)
            anyExternal = true;
        else if(d->hasInclude)
            anyHeader = true;

        if(d->isTranslationUnit())
            anySource = true;
    }
    if(anyExternal)
    {
        os << "\n  Exposed headers:";
        for(auto &d : component.files)
        {
            if(d->hasExternalInclude)
                os << " " << d->path.generic_string();
        }
    }
    if(anyHeader)
    {
        os << "\n  Private headers:";
        for(auto &d : component.files)
        {
            if(!d->hasExternalInclude && d->hasInclude)
                os << " " << d->path.generic_string();
        }
    }
    if(anySource)
    {
        os << "\n  Sources:\n";
        for(auto &d : component.files)
        {
            if(d->isTranslationUnit())
            {
                os << " " << d->path.generic_string();
                os << " (" << (d->moduleExported ? "export " : "") << d->moduleName << ")\n";
                for(auto &dep : d->dependencies)
                    os << "   " << dep.second->path.generic_string() << "\n";
            }
        }
    }
    if(!component.pubIncl.empty())
    {
        os << "\n  pubincl:\n";
        for(auto &d : component.pubIncl)
        {
            os << "    " << d << "\n";
        }
    }
    if(!component.privIncl.empty())
    {
        os << "\n  privincl:\n";
        for(auto &d : component.privIncl)
        {
            os << "    " << d << "\n";
        }
    }
    os << "\n  Commands to run:";
    for(auto &command : component.commands)
    {
        os << "\n"
           << *command;
    }

    os << "\n\n";
    return os;
}
