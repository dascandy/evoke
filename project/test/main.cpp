// mmap
#define BOOST_TEST_MODULE Evoke_project
#include "Component.h"

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/test/include/unit_test.hpp>

namespace btt = boost::test_tools;

BOOST_AUTO_TEST_CASE(mmap_file)
{
    filesystem::path path{"project/test/main.cpp"};

    using namespace boost::interprocess;
    file_mapping file(path.string().c_str(), read_only);
    mapped_region region(file, read_only);

    //Get the address of the mapped region
    void *addr = region.get_address();
    std::size_t size = region.get_size();

    size_t fileSize = filesystem::file_size(path);
    BOOST_TEST(fileSize == size);
    BOOST_TEST(strncmp(static_cast<char *>(addr), "// mmap", 7) == 0);
}

BOOST_AUTO_TEST_CASE(without_last_flash)
{
    Component c("./my/component", true);

    BOOST_TEST(c.name == "my_component");
}
