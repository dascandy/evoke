#include "test.h"
#include <iostream>

TEST_CASE("Compile a single file library with test", "[simple]") 
{
  /*
  testenvironment env;
  create("hello/src/hello.cpp", R"(
#include "hello.h"
int func() {
  return 2;
}
)");
  create("hello/include/hello.h", R"(
int func();
)");
  create("hello/test/testhello.cpp", R"(
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "hello.h"

TEST_CASE("Hello returns 4", "[hello]") {
  REQUIRE(func() == 4);
}
)");

  run_evoke("");
  REQUIRE(file_exists("build/linux/obj/hello/src/hello.cpp.o"));
  REQUIRE(file_exists("build/linux/lib/libhello.a"));
  REQUIRE(file_exists("build/linux/bin/hello_test"));
  REQUIRE(run("build/linux/bin/hello_test").second != 0);

  SECTION("And then fix the code") {
    create("hello/src/hello.cpp", R"(
#include "hello.h"
int func() {
  return 4;
}
)");
    uint64_t hashBefore = hash("build/linux/bin/hello_test");
    std::cout << run_evoke("").first;
    uint64_t hashAfter = hash("build/linux/bin/hello_test");
    REQUIRE(hashBefore != hashAfter);
    REQUIRE(run("build/linux/bin/hello_test").second == 0);
  }
  */
}


