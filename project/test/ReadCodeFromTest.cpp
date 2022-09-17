#include <boost/test/unit_test.hpp>
#include <string>
#include "../include/Project.h"
#include "../include/File.h"
BOOST_AUTO_TEST_CASE(ReadCodeFrom_one_single_include_statement)
{
    Component c(".");
    File f(".", c);
    std::string code = R"(
#include "myheader.h"
    )";
    Project::ReadCodeFrom(f, code.c_str(), code.size());
    BOOST_TEST(f.rawIncludes.size() == 1);
}

BOOST_AUTO_TEST_CASE(ReadCodeFrom_one_single_include_statement_crlf)
{
    Component c(".");
    File f(".", c);
    std::string code = "\n#include \"myheader.h\"\xa\xd";
    Project::ReadCodeFrom(f, code.c_str(), code.size());
    BOOST_TEST(f.rawIncludes.size() == 1);
}

BOOST_AUTO_TEST_CASE(ReadCodeFrom_one_single_import_statement)
{
    Component c("src");
    File f("test.cpp", c);
    std::string code = R"(
import myheader;
    )";
    Project::ReadCodeFrom(f, code.c_str(), code.size());
    BOOST_TEST(f.rawIncludes.size() == 0);
    BOOST_TEST(f.rawImports.size() == 0);
    BOOST_TEST(f.imports.count("myheader") != 0);
    BOOST_TEST(f.imports["myheader"] == false);
}

BOOST_AUTO_TEST_CASE(ReadCodeFrom_one_single_import_statement_crlf)
{
    Component c(".");
    File f(".", c);
    std::string code = "\nimport myheader;\xa\xd";
    Project::ReadCodeFrom(f, code.c_str(), code.size());
    BOOST_TEST(f.imports.count("myheader") != 0);
    BOOST_TEST(f.imports["myheader"] == false);
}

BOOST_AUTO_TEST_CASE(ReadCodeFrom_one_module_export_statement)
{
    Component c(".");
    File f(".", c);
    std::string code = R"(
export module mylib;
    )";
    Project::ReadCodeFrom(f, code.c_str(), code.size());
    BOOST_TEST(f.moduleExported == true);
    BOOST_TEST(f.moduleName == "mylib");
}

//export module mylib;
