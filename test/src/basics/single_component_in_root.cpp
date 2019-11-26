#include "test.h"

TEST_CASE("Compile a root component executable", "[simple]") 
{
  testenvironment env;
  create("src/hello.cpp", R"(
#include <stdio.h>
int main() {
  printf("Hello World!\n");
}
)");
  run_evoke("");
  REQUIRE(file_exists("build/linux/bin/test_temp"));
  REQUIRE(file_exists("build/linux/obj/src/hello.cpp.o"));
  REQUIRE(run("build/linux/bin/test_temp").first == "Hello World!\n");

  // Executable name is test_temp, because these unit tests run in a folder with that name.
}

