#include "Utilities.hpp"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(GetNameFromPath_anonymous)
{
    filesystem::path empty_path;
    BOOST_TEST(GetNameFromPath(empty_path) == "#anonymous#");
    BOOST_TEST(GetNameFromPath(empty_path, '/') == "#anonymous#");

    filesystem::path root(".");
    BOOST_TEST(GetNameFromPath(root) == "#anonymous#");
    BOOST_TEST(GetNameFromPath(root, '/') == "#anonymous#");

    filesystem::path root2("./");
    BOOST_TEST(GetNameFromPath(root2) == "#anonymous#");
    BOOST_TEST(GetNameFromPath(root2, '/') == "#anonymous#");
}

BOOST_AUTO_TEST_CASE(GetNameFromPath_first_level_component)
{
    filesystem::path component_path("./comp");
    BOOST_TEST(GetNameFromPath(component_path) == "comp");
    BOOST_TEST(GetNameFromPath(component_path, '/') == "comp");

    filesystem::path component_path2("comp");
    BOOST_TEST(GetNameFromPath(component_path2) == "comp");
    BOOST_TEST(GetNameFromPath(component_path2, '/') == "comp");
}

BOOST_AUTO_TEST_CASE(GetNameFromPath_second_level_component)
{
    filesystem::path subcomponent_path("./comp/subcomp");
    BOOST_TEST(GetNameFromPath(subcomponent_path) == "comp_subcomp");
    BOOST_TEST(GetNameFromPath(subcomponent_path, '/') == "comp/subcomp");

    filesystem::path subcomponent_path2("comp/subcomp");
    BOOST_TEST(GetNameFromPath(subcomponent_path2) == "comp_subcomp");
    BOOST_TEST(GetNameFromPath(subcomponent_path2, '/') == "comp/subcomp");
}

BOOST_AUTO_TEST_CASE(GetNameFromPath_third_level_component)
{
    filesystem::path subsub_path("./comp/subcomp/third");
    BOOST_TEST(GetNameFromPath(subsub_path) == "comp_subcomp_third");
    BOOST_TEST(GetNameFromPath(subsub_path, '/') == "comp/subcomp/third");

    filesystem::path subsub_path2("comp/subcomp/third");
    BOOST_TEST(GetNameFromPath(subsub_path2) == "comp_subcomp_third");
    BOOST_TEST(GetNameFromPath(subsub_path2, '/') == "comp/subcomp/third");
}