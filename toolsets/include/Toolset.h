#pragma once


#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

struct File;
struct Component;
class Project;

class Toolset
{
    virtual ~Toolset()
    {
    }
    virtual void CreateCommandsFor(Project &project, const std::vector<std::string> &targets) = 0;
    virtual void CreateCommandsForUnity(Project &project, const std::vector<std::string> &targets) = 0;
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
    void CreateCommandsForUnity(Project &project, const std::vector<std::string> &targets) override;
    void CreateCommandsFor(Project &project, const std::vector<std::string> &targets) override;
    void SetParameter(const std::string &key, const std::string &value) override;

protected:
    std::string GetCompilerFor(std::string extension);
    virtual std::string getUnityCommand(const std::string &program, const std::string &outputFile, const File *inputFile, const std::set<std::string> &includes, std::vector<std::vector<Component *>> linkDeps) = 0;
    virtual std::string getCompileCommand(const std::string &program, const std::string &outputFile, const File *inputFile, const std::set<std::string> &includes, bool usesModules) = 0;
    virtual std::string getPrecompileCommand(const std::string &program, const std::string &outputFile, const File *inputFile, const std::set<std::string> &includes, bool usesModules) = 0;
    virtual std::string getArchiverCommand(const std::string &program, const std::string &outputFile, const std::vector<File *> inputs) = 0;
    virtual std::string getLinkerCommand(const std::string &program, const std::string &outputFile, const std::vector<File *> objects, std::vector<std::vector<Component *>> linkDeps) = 0;
    virtual std::string getUnittestCommand(const std::string &program) = 0;
    std::map<std::string, std::string> parameters;
};

class ClangToolset : public GenericToolset
{
    ClangToolset();
    std::string getUnityCommand(const std::string &program, const std::string &outputFile, const File *inputFile, const std::set<std::string> &includes, std::vector<std::vector<Component *>> linkDeps) override;
    std::string getCompileCommand(const std::string &program, const std::string &outputFile, const File *inputFile, const std::set<std::string> &includes, bool hasModules) override;
    std::string getPrecompileCommand(const std::string &program, const std::string &outputFile, const File *inputFile, const std::set<std::string> &includes, bool hasModules) override;
    std::string getArchiverCommand(const std::string &program, const std::string &outputFile, const std::vector<File *> inputs) override;
    std::string getLinkerCommand(const std::string &program, const std::string &outputFile, const std::vector<File *> objects, std::vector<std::vector<Component *>> linkDeps) override;
    std::string getUnittestCommand(const std::string &program) override;
    std::string getBmiNameFor(const File &file) override;
    std::string getObjNameFor(const File &file) override;
    std::string getLibNameFor(const Component &component) override;
    std::string getExeNameFor(const Component &component) override;
};

class GccToolset : public GenericToolset
{
    GccToolset();
    std::string getUnityCommand(const std::string &program, const std::string &outputFile, const File *inputFile, const std::set<std::string> &includes, std::vector<std::vector<Component *>> linkDeps) override;
    std::string getCompileCommand(const std::string &program, const std::string &outputFile, const File *inputFile, const std::set<std::string> &includes, bool hasModules) override;
    std::string getPrecompileCommand(const std::string &program, const std::string &outputFile, const File *inputFile, const std::set<std::string> &includes, bool hasModules) override;
    std::string getArchiverCommand(const std::string &program, const std::string &outputFile, const std::vector<File *> inputs) override;
    std::string getLinkerCommand(const std::string &program, const std::string &outputFile, const std::vector<File *> objects, std::vector<std::vector<Component *>> linkDeps) override;
    std::string getUnittestCommand(const std::string &program) override;
    std::string getBmiNameFor(const File &file) override;
    std::string getObjNameFor(const File &file) override;
    std::string getLibNameFor(const Component &component) override;
    std::string getExeNameFor(const Component &component) override;
};

class MsvcToolset : public GenericToolset
{
    MsvcToolset();
    std::string getUnityCommand(const std::string &program, const std::string &outputFile, const File *inputFile, const std::set<std::string> &includes, std::vector<std::vector<Component *>> linkDeps) override;
    std::string getCompileCommand(const std::string &program, const std::string &outputFile, const File *inputFile, const std::set<std::string> &includes, bool hasModules) override;
    std::string getPrecompileCommand(const std::string &program, const std::string &outputFile, const File *inputFile, const std::set<std::string> &includes, bool hasModules) override;
    std::string getArchiverCommand(const std::string &program, const std::string &outputFile, const std::vector<File *> inputs) override;
    std::string getLinkerCommand(const std::string &program, const std::string &outputFile, const std::vector<File *> objects, std::vector<std::vector<Component *>> linkDeps) override;
    std::string getUnittestCommand(const std::string &program) override;
    std::string getBmiNameFor(const File &file) override;
    std::string getObjNameFor(const File &file) override;
    std::string getLibNameFor(const Component &component) override;
    std::string getExeNameFor(const Component &component) override;
};

std::unique_ptr<Toolset> GetToolsetByName(const std::string &name);
