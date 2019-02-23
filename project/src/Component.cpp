#include "Component.h"

#include "File.h"
#include "PendingCommand.h"

Component::Component(const filesystem::path &path, bool isBinary) :
    root(path),
    type("library"),
    isBinary(isBinary)
{
    std::string rp = path.string();
    if(rp[0] == '.' && (rp[1] == '/' || rp[1] == '\\'))
        rp = rp.substr(2);
    root = rp;
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
    if(root.string().empty())
        return filesystem::absolute(root).filename().string();
    return root.generic_string();
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
                os << " " << d->path.generic_string() << "\n";
                for(auto &dep : d->dependencies)
                    os << "   " << dep->path.generic_string() << "\n";
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
           << command->commandToRun;
    }

    os << "\n\n";
    return os;
}
