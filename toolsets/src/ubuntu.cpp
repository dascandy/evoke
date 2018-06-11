#include "Toolset.h"
#include "Component.h"
#include "PendingCommand.h"
#include "File.h"
#include "Project.h"
#include "filter.h"

template <bool usePrivDepsFromOthers = false>
struct Tarjan {
  // https://en.wikipedia.org/wiki/Tarjan%27s_strongly_connected_components_algorithm
  struct Info {
    size_t index;
    size_t lowlink;
    bool onStack = true;
  };

  Component* originalComponent;
  size_t index = 0;
  std::vector<Component*> stack;
  std::unordered_map<Component*, Info> info;
  std::vector<std::vector<Component*>> nodes;
  void StrongConnect(Component* c) {
    info[c].index = info[c].lowlink = index++;
    stack.push_back(c);
    for (auto& c2 : c->pubDeps) {
      if (info[c2].index == 0) {
        StrongConnect(c2);
        info[c].lowlink = std::min(info[c].lowlink, info[c2].lowlink);
      } else if (info[c2].onStack) {
        info[c].lowlink = std::min(info[c].lowlink, info[c2].index);
      }
    }
    bool shouldUsePrivDeps = (c == originalComponent) || usePrivDepsFromOthers;
    if (shouldUsePrivDeps) {
      for (auto& c2 : c->privDeps) {
        if (info[c2].index == 0) {
          StrongConnect(c2);
          info[c].lowlink = std::min(info[c].lowlink, info[c2].lowlink);
        } else if (info[c2].onStack) {
          info[c].lowlink = std::min(info[c].lowlink, info[c2].index);
        }
      }
    }
    if (info[c].lowlink == info[c].index) {
      auto it = std::find(stack.begin(), stack.end(), c);
      nodes.push_back(std::vector<Component*>(it, stack.end()));
      for (auto& ent : nodes.back()) {
        info[ent].onStack = false;
      }
      stack.resize(it - stack.begin());
    }
  }
};

static std::string getLibNameFor(Component& component) {
  // TODO: change commponent to dotted string before making
  return "lib" + component.root.string() + ".a";
}

static std::string getExeNameFor(Component& component) {
  if (component.root.string() != ".") {
    return component.root.string();
  }
  return boost::filesystem::canonical(component.root).filename().string();
}

std::vector<std::vector<Component*>> GetTransitiveAllDeps(Component& c) {
  Tarjan<true> t;
  t.originalComponent = &c;
  t.StrongConnect(&c);
  return t.nodes;
}

std::vector<std::vector<Component*>> GetTransitivePubDeps(Component& c) {
  Tarjan<false> t;
  t.originalComponent = &c;
  t.StrongConnect(&c);
  return t.nodes;
}

std::vector<Component*> flatten(std::vector<std::vector<Component*>> in) {
  std::vector<Component*> v;
  for (auto& p : in) {
    v.insert(v.end(), p.begin(), p.end());
  }
  return v;
}

std::set<std::string> getIncludePathsFor(Component& component) {
  std::vector<std::vector<Component*>> pdeps = GetTransitivePubDeps(component);
  std::set<std::string> inclpaths;
  for (auto& v : pdeps) {
    for (auto& c : v) {
      for (auto& p : c->pubIncl) {
        inclpaths.insert((c->root / p).string());
      }
    }
  }
  for (auto& p : component.pubIncl) {
    inclpaths.insert((component.root / p).string());
  }
  for (auto& p : component.privIncl) {
    inclpaths.insert((component.root / p).string());
  }
  return inclpaths;
}

void UbuntuToolset::CreateCommandsFor(Project& project, Component& component) {
  std::string includes;
  for (auto& d : getIncludePathsFor(component)) {
    includes += " -I" + d;
  }

  boost::filesystem::path outputFolder = component.root;
  std::vector<File*> objects;
  for (auto& f : filter(component.files, [&project](File*f){ return project.IsCompilationUnit(f->path.extension().string()); })) {
    boost::filesystem::path outputFile = std::string("obj") / outputFolder / (f->path.string().substr(component.root.string().size()) + ".o");
    File* of = project.CreateFile(component, outputFile);
    PendingCommand* pc = new PendingCommand("g++ -c -std=c++17 -o " + outputFile.string() + " " + f->path.string() + includes);
    objects.push_back(of);
    pc->AddOutput(of);
    std::unordered_set<File*> d;
    std::stack<File*> deps;
    deps.push(f);
    size_t index = 0;
    while (!deps.empty()) {
      File* dep = deps.top();
      deps.pop();
      pc->AddInput(dep);
      for (File* input : dep->dependencies)
        if (d.insert(input).second) deps.push(input);
      index++;
    }
    pc->Check();
    component.commands.push_back(pc);
  }
  if (!objects.empty()) {
    std::string command;
    boost::filesystem::path outputFile;
    PendingCommand* pc;
    if (component.type != "executable") {
      outputFile = "lib/" + getLibNameFor(component);
      command = "ar rcs " + outputFile.string();
      for (auto& file : objects) {
        command += " " + file->path.string();
      }
      pc = new PendingCommand(command);
    } else {
      outputFile = "bin/" + getExeNameFor(component);
      command = "g++ -pthread -o " + outputFile.string();

      for (auto& file : objects) {
        command += " " + file->path.string();
      }
      command += " -Llib";
      std::vector<std::vector<Component*>> linkDeps = GetTransitiveAllDeps(component);
      std::reverse(linkDeps.begin(), linkDeps.end());
      for (auto d : linkDeps) {
        size_t index = 0;
        while (index < d.size()) {
          if (d[index]->isHeaderOnly()) {
            d[index] = d.back();
            d.pop_back();
          } else {
            ++index;
          }
        }
        if (d.empty()) continue;
        if (d.size() == 1 || (d.size() == 2 && (d[0] == &component || d[1] == &component))) {
          if (d[0] != &component) {
            command += " -l" + d[0]->root.string();
          } else if (d.size() == 2) {
            command += " -l" + d[1]->root.string();
          }
        } else {
          command += " -Wl,--start-group";
          for (auto& c : d) {
            if (c != &component) {
              command += " -l" + c->root.string();
            }
          }
          command += " -Wl,--end-group";
        }
      }
      pc = new PendingCommand(command);
      for (auto& d : linkDeps) {
        for (auto& c : d) {
          if (c != &component) {
            pc->AddInput(project.CreateFile(*c, "lib/" + getLibNameFor(*c)));
          }
        }
      }
    }
    File* libraryFile = project.CreateFile(component, outputFile);
    pc->AddOutput(libraryFile);
    for (auto& file : objects) {
      pc->AddInput(file);
    }
    pc->Check();
    component.commands.push_back(pc);
  }
}


