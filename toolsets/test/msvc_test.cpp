#include "dotted.h"
#include "Component.h"
#include "File.h"
#include "Toolset.h"
#include "Project.h"
#include <algorithm>
#include <set>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(msvc_comile)
{
    Component c("hello", true);
    Project p("./");
    File *input = p.CreateFile(c, "hello/src/gretting.cpp");
    const std::set<std::string> includes {"hello\include"};
    MsvcToolset msvc;
    auto cmd = msvc.getCompileCommand("cl.exe", "/permissive- /std:c++latest", "obj/hello/src/gretting.cpp.obj", input, includes, false);
    BOOST_TEST(cmd == "cl.exe /c /EHsc /permissive- /std:c++latest /Foobj/hello/src/gretting.cpp.obj hello/src/gretting.cpp /Ihello\include");
}
BOOST_AUTO_TEST_CASE(msvc_archive)
{
    Component c("mylib", true);
    Project p("./");
    File *input = p.CreateFile(c, "obj/mylib/src/mylib.cpp.obj");
    MsvcToolset msvc;
    auto cmd = msvc.getArchiverCommand("lib.exe", "lib/libmylib.lib", {input});
    BOOST_TEST(cmd == "lib.exe /OUT:lib/libmylib.lib obj/mylib/src/mylib.cpp.obj");
} 
BOOST_AUTO_TEST_CASE(msvc_link)
{
    Component c("mylib", true);
    c.type = "library";
    c.buildSuccess = true;
    Project p("./");
    File *input1 = p.CreateFile(c, "obj/hello/src/gretting.cpp.obj");
    File *input2 = p.CreateFile(c, "obj/hello/src/main.cpp.obj");
    MsvcToolset msvc;
    auto cmd = msvc.getLinkerCommand("link.exe", "bin/hello.exe", {input1, input2}, {{&c}});
    BOOST_TEST(cmd == "link.exe /OUT:bin/hello.exe obj/hello/src/gretting.cpp.obj obj/hello/src/main.cpp.obj /LIBPATH:lib libmylib.lib");
}

