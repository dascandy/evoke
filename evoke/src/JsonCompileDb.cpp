#include "Project.h"
#include "JsonCompileDb.h"
#include <ostream>

static std::string escape(const std::string &str)
{
    std::string out;
    for(auto &c : str)
    {
        if(c == '"')
            out += "\\\"";
        else if(c == '\\')
            out += "\\\\";
        else
            out += c;
    }
    return out;
}

static void dumpPendingCommand(std::ostream &os, PendingCommand &pc)
{
    os << "{ \"directory\": \"" << filesystem::current_path().string() << "\",\n";
    os << "  \"command\": \"" << escape(pc.commandToRun) << "\", \n";
    os << "  \"file\": \"" << pc.inputs.front()->path.string() << "\", \n";
    os << "  \"output\": \"" << pc.outputs.front()->path.string() << "\" }";
}

void dumpJsonCompileDb(std::ostream &os, Project& op)
{
    os << "[\n";
    bool first = true;
    for(auto &c : op.components)
    {
        for(auto &pc : c.second.commands)
        {
            if(!first)
                os << ",\n";
            first = false;
            dumpPendingCommand(os, *pc);
        }
    }
    os << "]\n";
}
