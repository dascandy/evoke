#include "test.h"

TEST_CASE("Compile a single file executable", "[simple]") 
{
  testenvironment env;
  create("hello/src/hello.cpp", R"(
#include <stdio.h>
int main() {
  printf("Hello World!\n");
}
)");
  run_evoke("");
  REQUIRE(file_exists("build/linux/bin/hello"));
  REQUIRE(file_exists("build/linux/obj/hello/src/hello.cpp.o"));
  REQUIRE(run("build/linux/bin/hello").first == "Hello World!\n");
}

