#include "fw/dotted.h"

#include <algorithm>
#include <boost/test/unit_test.hpp>
namespace btt = boost::test_tools;

BOOST_AUTO_TEST_CASE(remove_dot)
{
    using fs::path;
    fs::create_directories("folder/filename");
    BOOST_TEST(removeDot("./folder/filename").generic_string() == "folder/filename");

    BOOST_TEST(removeDot("folder/filename").generic_string() == "folder/filename");
    fs::remove_all("folder");
}

BOOST_AUTO_TEST_CASE(as_dotted_works_for_paths)
{
    BOOST_TEST(as_dotted("root/component") == "root.component");
}


