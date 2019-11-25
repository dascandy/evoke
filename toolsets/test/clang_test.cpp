#include "fw/dotted.h"
#include "Component.h"
#include "File.h"
#include "Toolset.h"
#include "Project.h"
#include <algorithm>
#include <set>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(clang_compile)
{
    Component c("hello", true);
    Project p("./");
    File *input = p.CreateFile(c, "hello/src/gretting.cpp");
    const std::set<std::string> includes {"hello/include"};
    ClangToolset clang;
    auto cmd = clang.getCompileCommand("clang++", "obj/hello/src/gretting.cpp.obj", input, includes, false);
    BOOST_TEST(cmd == "clang++ -c -o obj/hello/src/gretting.cpp.obj hello/src/gretting.cpp -Ihello/include");
}

BOOST_AUTO_TEST_CASE(clang_archive)
{
    Component c("mylib", true);
    Project p("./");
    File *input = p.CreateFile(c, "obj/mylib/src/mylib.cpp.obj");
    ClangToolset clang;
    auto cmd = clang.getArchiverCommand("ar", "lib/libmylib.lib", {input});
    BOOST_TEST(cmd == "ar rcs lib/libmylib.lib obj/mylib/src/mylib.cpp.obj");
}

BOOST_AUTO_TEST_CASE(clang_link)
{
    Component c("mylib", true);
    c.type = "library";
    c.buildSuccess = true;
    Project p("./");
    File *input1 = p.CreateFile(c, "obj/hello/src/gretting.cpp.obj");
    File *input2 = p.CreateFile(c, "obj/hello/src/main.cpp.obj");
    ClangToolset clang;
    auto cmd = clang.getLinkerCommand("clang++", "bin/hello.exe", {input1, input2}, {{&c}});
    BOOST_TEST(cmd == "clang++ -o bin/hello.exe obj/hello/src/gretting.cpp.obj obj/hello/src/main.cpp.obj -Lbuild/clang/lib -lmylib");
}
