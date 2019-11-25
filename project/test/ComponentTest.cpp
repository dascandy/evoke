#include "Component.h"

#include <boost/test/unit_test.hpp>

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
