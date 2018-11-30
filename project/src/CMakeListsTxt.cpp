#include "utils.h"
#include "Configuration.h"
#include "Project.h"

#include <boost/filesystem/fstream.hpp>
#include <ostream>

struct CMakeProjectExporter
{
    explicit CMakeProjectExporter(const Project &project) :
        project_{project}
    {
    }

    std::string LookupLibraryName(const std::string &componentName)
    {
        return (project_.IsSystemComponent(componentName) ? cmakeSystemProjectPrefix + componentName : componentName);
    }

    void createCMakeListsFiles(const GlobalOptions &opts)
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
            boost::filesystem::path filePath = project_.projectRoot / comp.second.root / cmakelists_txt;
            boost::system::error_code ec;
            boost::filesystem::remove(filePath, ec);
            boost::filesystem::ofstream os{filePath};

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
                    auto relpath = boost::filesystem::relative(project_.projectRoot / file_entry->path,
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

            boost::filesystem::path filePath = project_.projectRoot / cmakelists_txt;
            boost::system::error_code ec;
            boost::filesystem::remove(filePath, ec);
            boost::filesystem::ofstream os{filePath};
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
                    os << "add_subdirectory(" << comp.second.GetName() << ")\n";
                }
            }
            for(const auto &comp : project_.components)
            {
                if(comp.second.type == "executable")
                {
                    os << "add_subdirectory(" << comp.second.GetName() << ")\n";
                }
            }
        }
    }

private:
    void dumpDoNotModifyWarning(std::ostream &os)
    {
        os << "# This is an autogenerated file, do not hand modify.\n";
        os << "# This file was generated by evoke to give cmake only IDEs\n";
        os << "# An idea about the project structure.\n\n";
    }

    void extractSystemComponents(const Component &comp, std::unordered_set<std::string> &visited, std::vector<const Component *> &results) const
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
        if(project_.IsSystemComponent(comp.GetName()))
        {
            results.push_back(&comp);
        }
    }

    std::vector<const Component *> extractSystemComponents() const
    {
        std::unordered_set<std::string> visited;
        std::vector<const Component *> systemComponents;
        for(const auto &comp : project_.components)
        {
            extractSystemComponents(comp.second, visited, systemComponents);
        }
        return systemComponents;
    }

    void dumpTargetIncludes(std::ostream &os, const std::string &target, const std::string &access, const std::unordered_set<std::string> &includes)
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

    void dumpTargetLibraries(std::ostream &os, const std::string &target, const std::string &access, const std::unordered_set<Component *> components)
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

    static constexpr const char *cmakeSystemProjectPrefix = "evoke_system_lib_";
    static constexpr const char *cmakelists_txt = "CMakeLists.txt";
    const Project &project_;
};

void Project::dumpCMakeListsTxt(const GlobalOptions &opts)
{
    CMakeProjectExporter exporter{*this};
    exporter.createCMakeListsFiles(opts);
}