#include "Component.h"
#include "File.h"
#include "PendingCommand.h"
#include "Project.h"
#include "Toolset.h"

template<bool usePrivDepsFromOthers = false>
struct Tarjan
{
    // https://en.wikipedia.org/wiki/Tarjan%27s_strongly_connected_components_algorithm
    struct Info
    {
        size_t index;
        size_t lowlink;
        bool onStack = true;
    };

    Component *originalComponent;
    size_t index = 0;
    std::vector<Component *> stack;
    std::unordered_map<Component *, Info> info;
    std::vector<std::vector<Component *>> nodes;
    void StrongConnect(Component *c)
    {
        info[c].index = info[c].lowlink = index++;
        stack.push_back(c);
        for(auto &c2 : c->pubDeps)
        {
            if(info[c2].index == 0)
            {
                StrongConnect(c2);
                info[c].lowlink = std::min(info[c].lowlink, info[c2].lowlink);
            }
            else if(info[c2].onStack)
            {
                info[c].lowlink = std::min(info[c].lowlink, info[c2].index);
            }
        }
        bool shouldUsePrivDeps = (c == originalComponent) || usePrivDepsFromOthers;
        if(shouldUsePrivDeps)
        {
            for(auto &c2 : c->privDeps)
            {
                if(info[c2].index == 0)
                {
                    StrongConnect(c2);
                    info[c].lowlink = std::min(info[c].lowlink, info[c2].lowlink);
                }
                else if(info[c2].onStack)
                {
                    info[c].lowlink = std::min(info[c].lowlink, info[c2].index);
                }
            }
        }
        if(info[c].lowlink == info[c].index)
        {
            auto it = std::find(stack.begin(), stack.end(), c);
            nodes.push_back(std::vector<Component *>(it, stack.end()));
            for(auto &ent : nodes.back())
            {
                info[ent].onStack = false;
            }
            stack.resize(it - stack.begin());
        }
    }
};

std::vector<std::vector<Component *>> GetTransitiveAllDeps(Component &c)
{
    Tarjan<true> t;
    t.originalComponent = &c;
    t.StrongConnect(&c);
    return t.nodes;
}

std::vector<std::vector<Component *>> GetTransitivePubDeps(Component &c)
{
    Tarjan<false> t;
    t.originalComponent = &c;
    t.StrongConnect(&c);
    return t.nodes;
}

std::set<std::string> getIncludePathsFor(Component &component)
{
    std::vector<std::vector<Component *>> pdeps = GetTransitivePubDeps(component);
    std::set<std::string> inclpaths;
    for(auto &v : pdeps)
    {
        for(auto &c : v)
        {
            for(auto &p : c->pubIncl)
            {
                if (p.front() == '/') {
                    inclpaths.insert(p);
                } else {
                    inclpaths.insert((c->root / p).string());
                }
            }
        }
    }
    for(auto &p : component.pubIncl)
    {
        inclpaths.insert((component.root / p).string());
    }
    for(auto &p : component.privIncl)
    {
        inclpaths.insert((component.root / p).string());
    }
    return inclpaths;
}
