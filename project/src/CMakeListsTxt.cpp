#include "Configuration.h"
#include "Project.h"
#include "globaloptions.h"
#include "CMakeListsTxt.h"
#include <fw/filesystem.hpp>
#include <ostream>

CMakeProjectExporter::CMakeProjectExporter(const Project &project) :
    project_{project}
{
}

std::string CMakeProjectExporter::LookupLibraryName(const Component &comp)
{
    return (project_.IsSystemComponent(comp) ? cmakeSystemProjectPrefix + comp.GetName() : comp.GetName());
}

void CMakeProjectExporter::createCMakeListsFiles(const GlobalOptions &opts)
{
    auto injectOptions = [](std::ostream &os, const std::string &target_option, const std::string &target, const std::string &access, const std::vector<std::string> &opts) {
        if(!opts.empty())
        {
            os << target_option << "(" << target << " " << access;
            for(const auto &opt : opts)
            {
                os << " " << opt;
            }
            os << ")\n";
        }
    };

    for(const auto &comp : project_.components)
    {
        filesystem::path filePath = project_.projectRoot / comp.second.root / cmakelists_txt;
        filesystem::error_code ec;
        filesystem::remove(filePath, ec);
        filesystem::ofstream os{filePath};

        std::string target = comp.second.GetName();

        dumpDoNotModifyWarning(os);

        if(comp.second.type != "library" && comp.second.type != "executable")
        {
            continue;
        }
        std::string target_type = "add_executable(";
        std::string target_modifier;
        std::string publicAccess = "PUBLIC";
        std::string privateAccess = "PRIVATE";

        if(comp.second.type == "library")
        {
            target_type = "add_library(";
            target_modifier = " STATIC";

            if(comp.second.isHeaderOnly())
            {
                target_modifier = " INTERFACE";
                publicAccess = "INTERFACE";
                // No header only lib should have a private
                // dependency.  If it does, make it an public/interface.
                privateAccess = "INTERFACE";
            }
        }
        os << target_type << target << target_modifier << "\n";
        if(!comp.second.isHeaderOnly())
        {
            for(const auto &file_entry : comp.second.files)
            {
                auto relpath = filesystem::relative(project_.projectRoot / file_entry->path,
                                                    comp.second.root);
                os << "    " << relpath.string() << "\n";
            }
        }
        os << ")\n\n";
        injectOptions(os, "target_compile_options", target, privateAccess, opts.compile);
        injectOptions(os, "target_include_directories", target, privateAccess, opts.include);
        dumpTargetIncludes(os, target, publicAccess, comp.second.pubIncl);
        dumpTargetIncludes(os, target, privateAccess, comp.second.privIncl);
        injectOptions(os, "target_link_libraries", target, privateAccess, opts.link);
        dumpTargetLibraries(os, target, publicAccess, comp.second.pubDeps);
        dumpTargetLibraries(os, target, privateAccess, comp.second.privDeps);
    }

    {
        std::vector<const Component *> systemComponents = extractSystemComponents();

        filesystem::path filePath = project_.projectRoot / cmakelists_txt;
        filesystem::error_code ec;
        filesystem::remove(filePath, ec);
        filesystem::ofstream os{filePath};
        os << "cmake_minimum_required(VERSION 3.12)\n";
        os << "project(" << project_.projectRoot.filename().string() << ")\n\n";

        dumpDoNotModifyWarning(os);

        if(!systemComponents.empty())
        {
            os << "# System components\n";
            for(const auto &comp : systemComponents)
            {
                auto target = LookupLibraryName(*comp);
                if(target == comp->GetName())
                {
                    std::cout << "Error " << target << " = " << comp->GetName() << "\n";
                    exit(1);
                }
                os << "add_library(" << target << " INTERFACE)\n";
                if(!comp->pubDeps.empty() || !comp->isHeaderOnly())
                {
                    os << "target_link_libraries(" << target << " INTERFACE\n";
                    if(!comp->isHeaderOnly())
                    {
                        os << "    " << comp->GetName() << "\n";
                    }
                    for(const auto &subComp : comp->pubDeps)
                    {
                        os << "    " << LookupLibraryName(*subComp);
                    }
                    os << ")\n";
                }
            }
            os << "\n";
        }

        for(const auto &comp : project_.components)
        {
            if(comp.second.type == "library")
            {
                os << "add_subdirectory(" << comp.second.GetHierarchicalName() << ")\n";
            }
        }
        for(const auto &comp : project_.components)
        {
            if(comp.second.type == "executable")
            {
                os << "add_subdirectory(" << comp.second.GetHierarchicalName() << ")\n";
            }
        }
    }
}

void CMakeProjectExporter::dumpDoNotModifyWarning(std::ostream &os)
{
    os << "# This is an autogenerated file, do not hand modify.\n";
    os << "# This file was generated by evoke to give cmake only IDEs\n";
    os << "# An idea about the project structure.\n\n";
}

void CMakeProjectExporter::extractSystemComponents(const Component &comp, std::unordered_set<std::string> &visited, std::vector<const Component *> &results) const
{
    auto inserted = visited.insert(comp.GetName());
    if(!inserted.second)
    {
        return;
    }
    for(const auto &subComp : comp.pubDeps)
    {
        if(!subComp)
        {
            continue;
        }
        extractSystemComponents(*subComp, visited, results);
    }
    for(const auto &subComp : comp.privDeps)
    {
        if(!subComp)
        {
            continue;
        }
        extractSystemComponents(*subComp, visited, results);
    }
    if(project_.IsSystemComponent(comp))
    {
        results.push_back(&comp);
    }
}

std::vector<const Component *> CMakeProjectExporter::extractSystemComponents() const
{
    std::unordered_set<std::string> visited;
    std::vector<const Component *> systemComponents;
    for(const auto &comp : project_.components)
    {
        extractSystemComponents(comp.second, visited, systemComponents);
    }
    return systemComponents;
}

void CMakeProjectExporter::dumpTargetIncludes(std::ostream &os, const std::string &target, const std::string &access, const std::unordered_set<std::string> &includes)
{
    if(!includes.empty())
    {
        os << "target_include_directories(" << target << " " << access << "\n";
        for(const auto &incl : includes)
        {
            os << "    ${CMAKE_CURRENT_SOURCE_DIR}/" << incl << "\n";
        }
        os << ")\n";
    }
}

void CMakeProjectExporter::dumpTargetLibraries(std::ostream &os, const std::string &target, const std::string &access, const std::unordered_set<Component *> components)
{
    if(!components.empty())
    {
        os << "target_link_libraries(" << target << " " << access << "\n";
        for(const auto &comp : components)
        {
            os << "    " << LookupLibraryName(*comp) << "\n";
        }
        os << ")\n";
    }
}

