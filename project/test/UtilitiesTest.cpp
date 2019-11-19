#include "Utilities.hpp"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(GetNameFromPath_anonymous)
{
    fs::path empty_path;
    BOOST_TEST(GetNameFromPath(empty_path) == "evoke");
    BOOST_TEST(GetNameFromPath(empty_path, '/') == "evoke");

    fs::path root(".");
    BOOST_TEST(GetNameFromPath(root) == "evoke");
    BOOST_TEST(GetNameFromPath(root, '/') == "evoke");

    fs::path root2("./");
    BOOST_TEST(GetNameFromPath(root2) == "evoke");
    BOOST_TEST(GetNameFromPath(root2, '/') == "evoke");
}

BOOST_AUTO_TEST_CASE(GetNameFromPath_first_level_component)
{
    fs::path component_path("./comp");
    BOOST_TEST(GetNameFromPath(component_path) == "comp");
    BOOST_TEST(GetNameFromPath(component_path, '/') == "comp");

    fs::path component_path2("comp");
    BOOST_TEST(GetNameFromPath(component_path2) == "comp");
    BOOST_TEST(GetNameFromPath(component_path2, '/') == "comp");
}

BOOST_AUTO_TEST_CASE(GetNameFromPath_second_level_component)
{
    fs::path subcomponent_path("./comp/subcomp");
    BOOST_TEST(GetNameFromPath(subcomponent_path) == "comp_subcomp");
    BOOST_TEST(GetNameFromPath(subcomponent_path, '/') == "comp/subcomp");

    fs::path subcomponent_path2("comp/subcomp");
    BOOST_TEST(GetNameFromPath(subcomponent_path2) == "comp_subcomp");
    BOOST_TEST(GetNameFromPath(subcomponent_path2, '/') == "comp/subcomp");
}

BOOST_AUTO_TEST_CASE(GetNameFromPath_third_level_component)
{
    fs::path subsub_path("./comp/subcomp/third");
    BOOST_TEST(GetNameFromPath(subsub_path) == "comp_subcomp_third");
    BOOST_TEST(GetNameFromPath(subsub_path, '/') == "comp/subcomp/third");

    fs::path subsub_path2("comp/subcomp/third");
    BOOST_TEST(GetNameFromPath(subsub_path2) == "comp_subcomp_third");
    BOOST_TEST(GetNameFromPath(subsub_path2, '/') == "comp/subcomp/third");
}