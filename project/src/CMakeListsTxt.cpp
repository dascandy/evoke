#include "Project.h"
#include "CMakeListsTxt.h"
#include "Utilities.hpp"
#include <fw/filesystem.hpp>
#include <ostream>
#include <fstream>

CMakeProjectExporter::CMakeProjectExporter(const Project &project) :
    project_{project}
{
}

std::string CMakeProjectExporter::LookupLibraryName(const std::string &componentName)
{
    return (IsSystemComponent(componentName) ? cmakeSystemProjectPrefix + componentName : componentName);
}

void CMakeProjectExporter::createCMakeListsFiles(const Toolset &)
{
    for(const auto &comp : project_.components)
    {
        fs::path filePath = project_.projectRoot / comp.second.root / cmakelists_txt;
        fs::error_code ec;
        fs::remove(filePath, ec);
        std::ofstream os{filePath};

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
                auto relpath = fs::relative(project_.projectRoot / file_entry->path,
                                                    comp.second.root);
                os << "    " << relpath.string() << "\n";
            }
        }
        os << ")\n\n";
//    auto injectOptions = [](std::ostream &os, const std::string &target_option, const std::string &target, const std::string &access, const std::vector<std::string> &opts) {
//        if(!opts.empty())
//        {
//            os << target_option << "(" << target << " " << access;
//            for(const auto &opt : opts)
//            {
//                os << " " << opt;
//            }
//            os << ")\n";
//        }
//    };

//        injectOptions(os, "target_compile_options", target, privateAccess, opts.compile);
//        injectOptions(os, "target_include_directories", target, privateAccess, opts.include);
        dumpTargetIncludes(os, target, publicAccess);
//        injectOptions(os, "target_link_libraries", target, privateAccess, opts.link);
        dumpTargetLibraries(os, target, publicAccess, comp.second.pubDeps);
        dumpTargetLibraries(os, target, privateAccess, comp.second.privDeps);
    }

    {
        std::vector<const Component *> systemComponents = extractSystemComponents();

        fs::path filePath = project_.projectRoot / cmakelists_txt;
        fs::error_code ec;
        fs::remove(filePath, ec);
        std::ofstream os{filePath};
        os << "cmake_minimum_required(VERSION 3.12)\n";
        os << "project(" << project_.projectRoot.filename().string() << ")\n\n";

        dumpDoNotModifyWarning(os);

        if(!systemComponents.empty())
        {
            os << "# System components\n";
            for(const auto &comp : systemComponents)
            {
                auto target = LookupLibraryName(comp->GetName());
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
                        os << "    " << LookupLibraryName(subComp->GetName());
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
                os << "add_subdirectory(" << hierarchical_names[comp.second.GetName()] << ")\n";
            }
        }
        for(const auto &comp : project_.components)
        {
            if(comp.second.type == "executable")
            {
                os << "add_subdirectory(" << hierarchical_names[comp.second.GetName()] << ")\n";
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
    if(IsSystemComponent(comp.GetName()))
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

void CMakeProjectExporter::dumpTargetIncludes(std::ostream &os, const std::string &target, const std::string &access)
{
    os << "target_include_directories(" << target << " " << access << "\n";
    os << "    ${CMAKE_CURRENT_SOURCE_DIR}/include\n";
    os << ")\n";
}

void CMakeProjectExporter::dumpTargetLibraries(std::ostream &os, const std::string &target, const std::string &access, const std::unordered_set<Component *> components)
{
    if(!components.empty())
    {
        os << "target_link_libraries(" << target << " " << access << "\n";
        for(const auto &comp : components)
        {
            os << "    " << LookupLibraryName(comp->GetName()) << "\n";
        }
        os << ")\n";
    }
}

void CMakeProjectExporter::extractHierarchicalNames()
{
    for(const auto &comp : project_.components)
    {
        hierarchical_names.emplace(comp.second.GetName(), GetNameFromPath(comp.second.root, '/'));
    }
}

bool CMakeProjectExporter::IsSystemComponent(const std::string &name) const
{
    if (hierarchical_names.find(name) != hierarchical_names.cend())
    {
        return false;
    }
    auto altName = "./" + name;
    if (hierarchical_names.find(altName) != hierarchical_names.cend())
    {
        return false;
    }
    return true;
}
