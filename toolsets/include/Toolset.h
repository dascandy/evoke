#pragma once

#include "globaloptions.h"

#include <memory>
#include <set>
#include <string>
#include <vector>

struct File;
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
    virtual void SetParameter(const std::string &key, const std::string &value) = 0;
    virtual std::string getBmiNameFor(const File &file) = 0;
    virtual std::string getObjNameFor(const File &file) = 0;
    virtual std::string getLibNameFor(const Component &component) = 0;
    virtual std::string getExeNameFor(const Component &component) = 0;
    std::string getNameFor(const Component &component);
};

class GenericToolset : public Toolset
{
public:
    void CreateCommandsForUnity(Project &project) override;
    void CreateCommandsFor(Project &project) override;

protected:
    virtual std::string getUnityCommand(const std::string &program, const std::string &compileFlags, const std::string &outputFile, const File *inputFile, const std::set<std::string> &includes, std::vector<std::vector<Component *>> linkDeps) = 0;
    virtual std::string getCompileCommand(const std::string &program, const std::string &compileFlags, const std::string &outputFile, const File *inputFile, const std::set<std::string> &includes, bool usesModules) = 0;
    virtual std::string getPrecompileCommand(const std::string &program, const std::string &compileFlags, const std::string &outputFile, const File *inputFile, const std::set<std::string> &includes, bool usesModules) = 0;
    virtual std::string getArchiverCommand(const std::string &program, const std::string &outputFile, const std::vector<File *> inputs) = 0;
    virtual std::string getLinkerCommand(const std::string &program, const std::string &outputFile, const std::vector<File *> objects, std::vector<std::vector<Component *>> linkDeps) = 0;
    virtual std::string getUnittestCommand(const std::string &program) = 0;
    std::string compiler;
    std::string linker;
    std::string archiver;
};

struct AndroidToolset : public Toolset
{
    void CreateCommandsFor(Project &project) override;
    void CreateCommandsForUnity(Project &project) override;
    void SetParameter(const std::string &key, const std::string &value) override;
    std::string getBmiNameFor(const File &file) override;
    std::string getObjNameFor(const File &file) override;
    std::string getLibNameFor(const Component &component) override;
    std::string getExeNameFor(const Component &component) override;
};

struct ClangToolset : public GenericToolset
{
    ClangToolset();
    GlobalOptions ParseGeneralOptions(const std::string &options) override;
    void SetParameter(const std::string &key, const std::string &value) override;
    std::string getUnityCommand(const std::string &program, const std::string &compileFlags, const std::string &outputFile, const File *inputFile, const std::set<std::string> &includes, std::vector<std::vector<Component *>> linkDeps) override;
    std::string getCompileCommand(const std::string &program, const std::string &compileFlags, const std::string &outputFile, const File *inputFile, const std::set<std::string> &includes, bool hasModules) override;
    std::string getPrecompileCommand(const std::string &program, const std::string &compileFlags, const std::string &outputFile, const File *inputFile, const std::set<std::string> &includes, bool hasModules) override;
    std::string getArchiverCommand(const std::string &program, const std::string &outputFile, const std::vector<File *> inputs) override;
    std::string getLinkerCommand(const std::string &program, const std::string &outputFile, const std::vector<File *> objects, std::vector<std::vector<Component *>> linkDeps) override;
    std::string getUnittestCommand(const std::string &program) override;
    std::string getBmiNameFor(const File &file) override;
    std::string getObjNameFor(const File &file) override;
    std::string getLibNameFor(const Component &component) override;
    std::string getExeNameFor(const Component &component) override;
};

struct GccToolset : public GenericToolset
{
    GccToolset();
    GlobalOptions ParseGeneralOptions(const std::string &options) override;
    void SetParameter(const std::string &key, const std::string &value) override;
    std::string getUnityCommand(const std::string &program, const std::string &compileFlags, const std::string &outputFile, const File *inputFile, const std::set<std::string> &includes, std::vector<std::vector<Component *>> linkDeps) override;
    std::string getCompileCommand(const std::string &program, const std::string &compileFlags, const std::string &outputFile, const File *inputFile, const std::set<std::string> &includes, bool hasModules) override;
    std::string getPrecompileCommand(const std::string &program, const std::string &compileFlags, const std::string &outputFile, const File *inputFile, const std::set<std::string> &includes, bool hasModules) override;
    std::string getArchiverCommand(const std::string &program, const std::string &outputFile, const std::vector<File *> inputs) override;
    std::string getLinkerCommand(const std::string &program, const std::string &outputFile, const std::vector<File *> objects, std::vector<std::vector<Component *>> linkDeps) override;
    std::string getUnittestCommand(const std::string &program) override;
    std::string getBmiNameFor(const File &file) override;
    std::string getObjNameFor(const File &file) override;
    std::string getLibNameFor(const Component &component) override;
    std::string getExeNameFor(const Component &component) override;
};

struct MsvcToolset : public GenericToolset
{
    MsvcToolset();
    void SetParameter(const std::string &key, const std::string &value) override;
    std::string getUnityCommand(const std::string &program, const std::string &compileFlags, const std::string &outputFile, const File *inputFile, const std::set<std::string> &includes, std::vector<std::vector<Component *>> linkDeps) override;
    std::string getCompileCommand(const std::string &program, const std::string &compileFlags, const std::string &outputFile, const File *inputFile, const std::set<std::string> &includes, bool hasModules) override;
    std::string getPrecompileCommand(const std::string &program, const std::string &compileFlags, const std::string &outputFile, const File *inputFile, const std::set<std::string> &includes, bool hasModules) override;
    std::string getArchiverCommand(const std::string &program, const std::string &outputFile, const std::vector<File *> inputs) override;
    std::string getLinkerCommand(const std::string &program, const std::string &outputFile, const std::vector<File *> objects, std::vector<std::vector<Component *>> linkDeps) override;
    std::string getUnittestCommand(const std::string &program) override;
    std::string getBmiNameFor(const File &file) override;
    std::string getObjNameFor(const File &file) override;
    std::string getLibNameFor(const Component &component) override;
    std::string getExeNameFor(const Component &component) override;
};

std::unique_ptr<Toolset> GetToolsetByName(const std::string &name);
