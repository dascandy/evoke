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
    virtual GlobalOptions ParseGeneralOptions(const std::string &options);
    virtual void SetParameter(const std::string& key, const std::string& value) = 0;
};

struct AndroidToolset : public Toolset
{
    void CreateCommandsFor(Project &project) override;
    void SetParameter(const std::string& key, const std::string& value) override;
};

struct ClangToolset : public Toolset
{
    ClangToolset();
    void CreateCommandsFor(Project &project) override;
    GlobalOptions ParseGeneralOptions(const std::string &options) override;
    void SetParameter(const std::string& key, const std::string& value) override;
    std::string compiler, linker, archiver;
};

struct GccToolset : public Toolset
{
    GccToolset();
    void CreateCommandsFor(Project &project) override;
    GlobalOptions ParseGeneralOptions(const std::string &options) override;
    void SetParameter(const std::string& key, const std::string& value) override;
    std::string compiler, linker, archiver;
};

struct MsvcToolset : public Toolset
{
    MsvcToolset();
    void CreateCommandsFor(Project &project) override;
    void SetParameter(const std::string& key, const std::string& value) override;
    std::string compiler, linker, archiver;
};

std::unique_ptr<Toolset> GetToolsetByName(const std::string &name);
