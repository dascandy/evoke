#pragma once

#include "globaloptions.h"

#include <memory>
#include <string>
#include <vector>

struct Component;
class Project;

struct Toolset
{
    virtual ~Toolset()
    {
    }
    virtual void CreateCommandsFor(Project &project) = 0;
    virtual void CreateCommandsForUnity(Project &project) = 0;
    virtual GlobalOptions ParseGeneralOptions(const std::string &options);
    virtual void SetParameter(const std::string& key, const std::string& value) = 0;
    virtual std::string getLibNameFor(Component &component) = 0;
    virtual std::string getExeNameFor(Component &component) = 0;
    std::string getNameFor(Component &component);
};

struct AndroidToolset : public Toolset
{
    void CreateCommandsFor(Project &project) override;
    void CreateCommandsForUnity(Project &project) override;
    void SetParameter(const std::string& key, const std::string& value) override;
    std::string getLibNameFor(Component &component) override;
    std::string getExeNameFor(Component &component) override;
};

struct ClangToolset : public Toolset
{
    ClangToolset();
    void CreateCommandsFor(Project &project) override;
    void CreateCommandsForUnity(Project &project) override;
    GlobalOptions ParseGeneralOptions(const std::string &options) override;
    void SetParameter(const std::string& key, const std::string& value) override;
    std::string getLibNameFor(Component &component) override;
    std::string getExeNameFor(Component &component) override;
    std::string compiler, linker, archiver;
};

struct GccToolset : public Toolset
{
    GccToolset();
    void CreateCommandsFor(Project &project) override;
    void CreateCommandsForUnity(Project &project) override;
    GlobalOptions ParseGeneralOptions(const std::string &options) override;
    void SetParameter(const std::string& key, const std::string& value) override;
    std::string getLibNameFor(Component &component) override;
    std::string getExeNameFor(Component &component) override;
    std::string compiler, linker, archiver;
};

struct MsvcToolset : public Toolset
{
    MsvcToolset();
    void CreateCommandsFor(Project &project) override;
    void CreateCommandsForUnity(Project &project) override;
    void SetParameter(const std::string& key, const std::string& value) override;
    std::string getLibNameFor(Component &component) override;
    std::string getExeNameFor(Component &component) override;
    std::string compiler, linker, archiver;
};

std::unique_ptr<Toolset> GetToolsetByName(const std::string &name);
