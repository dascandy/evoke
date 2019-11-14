#include "test.h"
#include <iostream>
/*
TEST_CASE("Compile a single file library with test", "[simple]") 
{
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
  REQUIRE(file_exists("build/gcc/obj/hello/src/hello.cpp.o"));
  REQUIRE(file_exists("build/gcc/lib/libhello.a"));
  REQUIRE(file_exists("build/gcc/bin/hello_test"));
  REQUIRE(run("build/gcc/bin/hello_test").second != 0);

  SECTION("And then fix the code") {
    create("hello/src/hello.cpp", R"(
#include "hello.h"
int func() {
  return 4;
}
)");
    uint64_t hashBefore = hash("build/gcc/bin/hello_test");
    run_evoke("");
    uint64_t hashAfter = hash("build/gcc/bin/hello_test");
    REQUIRE(hashBefore != hashAfter);
    REQUIRE(run("build/gcc/bin/hello_test").second == 0);
  }
}
*/

