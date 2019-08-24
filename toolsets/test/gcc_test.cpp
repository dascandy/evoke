#include "dotted.h"
#include "Component.h"
#include "File.h"
#include "Toolset.h"
#include "Project.h"
#include <algorithm>
#include <set>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(gcc_compile)
{
    Component c("hello", true);
    Project p("./");
    File *input = p.CreateFile(c, "hello/src/gretting.cpp");
    const std::set<std::string> includes {"hello/include"};
    GccToolset gcc;
    auto cmd = gcc.getCompileCommand("g++", "-std=c++17", "obj/hello/src/gretting.cpp.obj", input, includes, false);
    BOOST_TEST(cmd == "g++ -c -std=c++17 -o obj/hello/src/gretting.cpp.obj hello/src/gretting.cpp -Ihello/include");
}

BOOST_AUTO_TEST_CASE(gcc_archive)
{
    Component c("mylib", true);
    Project p("./");
    File *input = p.CreateFile(c, "obj/mylib/src/mylib.cpp.obj");
    GccToolset gcc;
    auto cmd = gcc.getArchiverCommand("ar", "lib/libmylib.lib", {input});
    BOOST_TEST(cmd == "ar rcs lib/libmylib.lib obj/mylib/src/mylib.cpp.obj");
}

BOOST_AUTO_TEST_CASE(gcc_link)
{
    Component c("mylib", true);
    c.type = "library";
    c.buildSuccess = true;
    Project p("./");
    File *input1 = p.CreateFile(c, "obj/hello/src/gretting.cpp.obj");
    File *input2 = p.CreateFile(c, "obj/hello/src/main.cpp.obj");
    GccToolset gcc;
    auto cmd = gcc.getLinkerCommand("g++", "bin/hello.exe", {input1, input2}, {{&c}});
    BOOST_TEST(cmd == "g++ -pthread -o bin/hello.exe obj/hello/src/gretting.cpp.obj obj/hello/src/main.cpp.obj -Lbuild/gcc/lib -lmylib");
}
