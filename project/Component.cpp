#include "Component.h"
#include "File.h"
#include "PendingCommand.h"

Component::Component(const boost::filesystem::path &path)
        : root(path), type("library") {
  std::string rp = path.string();
  if (rp[0] == '.' && rp[1] == '/') rp = rp.substr(2);
  root = rp;
}

std::string Component::GetName() const {
    if (root.size() < 1)
        return boost::filesystem::absolute(root).filename().string();
    return root.generic_string();
}

std::ostream& operator<<(std::ostream& os, const Component& component) {
    if (component.files.empty()) return os;
    os << "Component " << component.GetName() << " built as " << component.type;
    if (!component.pubDeps.empty()) {
      os << "\n  Exposes:";
      for (auto& d : component.pubDeps) {
          os << " " << d->GetName();
      }
    }
    if (!component.privDeps.empty()) {
      os << "\n  Uses:";
      for (auto& d : component.privDeps) {
          os << " " << d->GetName();
      }
    }
    bool anyExternal = false, anyHeader = false, anySource = false;
    for (auto& d : component.files) {
        if (d->hasExternalInclude) anyExternal = true;
        else if (d->hasInclude) anyHeader = true;
        else anySource = true;
    }
    if (anyExternal) {
        os << "\n  Exposed headers:";
        for (auto& d : component.files) {
            if (d->hasExternalInclude) 
                os << " " << d->path.generic_string();
        }
    }
    if (anyHeader) {
        os << "\n  Private headers:";
        for (auto& d : component.files) {
            if (!d->hasExternalInclude && d->hasInclude) 
                os << " " << d->path.generic_string();
        }
    }
    if (anySource) {
        os << "\n  Sources:";
        for (auto& d : component.files) {
            if (!d->hasInclude) 
                os << " " << d->path.generic_string();
        }
    }
    os << "\n  Commands to run:";
    for (auto& command : component.commands) {
      os << "\n" << command->commandToRun;
    }

    os << "\n\n";
    return os;
}


