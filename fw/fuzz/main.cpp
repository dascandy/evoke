#include "fw/FileParser.h"
#include <fstream>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  std::ofstream("tmp.dat").write((const char*)Data, Size);
  ParseFile("tmp.dat", [](auto){}, [](auto, auto) {});
  return 0;
}


