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
