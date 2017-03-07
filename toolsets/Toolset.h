#pragma once

struct PendingCommand {
  std::vector<File*> inputs;
  std::vector<File*> dependencies;
  std::vector<File*> outputs;
  std::string command;
};

struct Toolset {
  virtual PendingCommand* createCommand(File& inputFile, const boost::filesystem::path& outputFolder) = 0;
  virtual PendingCommand* createLinkCommand(const std::vector<File*> &inputFiles, const boost::filesystem::path& outputFile) = 0;
};

/*

1. generate sources from inputs
2. compile sources to object files
3. link object files to static/shared library file

*/
