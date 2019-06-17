// mmap
#define BOOST_TEST_MODULE Evoke_project
#include "Component.h"

#include <boost/test/included/unit_test.hpp>

namespace btt = boost::test_tools;

BOOST_AUTO_TEST_CASE(without_last_slash)
{
    Component c("./my/component", true);

    BOOST_TEST(c.name == "my_component");
}

BOOST_AUTO_TEST_CASE(with_last_slash)
{
    Component c("./my/component/", true);

    BOOST_TEST(c.name == "my_component");
}

BOOST_AUTO_TEST_CASE(cmake_subdir_depth_1_without_last_slash)
{
    Component c("./component", true);

    BOOST_TEST(c.GetHierarchicalName() == "component");
}

BOOST_AUTO_TEST_CASE(cmake_subdir_depth_1_with_last_slash)
{
    Component c("./component/", true);

    BOOST_TEST(c.GetHierarchicalName() == "component");
}

BOOST_AUTO_TEST_CASE(cmake_subdir_depth_2_without_last_slash)
{
    Component c("./comp/subcomp", true);

    BOOST_TEST(c.GetHierarchicalName() == "comp/subcomp");
}

BOOST_AUTO_TEST_CASE(cmake_subdir_depth_2_with_last_slash)
{
    Component c("./comp/subcomp/", true);

    BOOST_TEST(c.GetHierarchicalName() == "comp/subcomp");
}
