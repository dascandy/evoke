#include "ubuntu.h"

std::unique_ptr<PendingCommand> UbuntuToolset::createCommand(File& inputFile, const boost::filesystem::path& outputFolder) {
  std::string command = "g++ -c -o " + outputFile.generic_string() + " " + inputFile.path.generic_string() + " -Wall -Wextra -Wpedantic";
  boost::filesystem::path outputFile = outputFolder / (inputFile.filename() + ".o");
  return std::make_unique<PendingCommand>(std::move(command), std::vector<File*>{inputFile}, std::vector<File*>{}, std::vector<File*>{outputFile});
}

std::unique_ptr<PendingCommand> UbuntuToolset::createLinkCommand(std::vector<File*> &&inputFiles, const boost::filesystem::path& outputFile) {
  std::string command = "g++ -o " + outputFile.generic_string();
  for (auto& input : inputFiles) {
    command += " " + input.path.generic_string();
  std::vector<File*> outputs;
  outputs.push_back(outputFile);
  return std::make_unique<PendingCommand>(std::move(command), inputFiles, std::vector<File*>{}, std::move(outputs));
}






