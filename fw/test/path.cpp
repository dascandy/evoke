#include <algorithm>
#include <boost/test/unit_test.hpp>
namespace btt = boost::test_tools;

#include "fw/filesystem.hpp"

BOOST_AUTO_TEST_CASE(path_prefix)
{
    using fs::path;

    path base{"./some_component/"};

    BOOST_TEST(base.begin()->filename_is_dot());

    BOOST_TEST(!path("some/sub/").begin()->filename_is_dot());
}

BOOST_AUTO_TEST_CASE(path_manipulate)
{
    using fs::path;

    path base{"obj/"};

    path file{base};
    file /= "./file";

    BOOST_TEST(file.string() == "obj/./file");
    BOOST_TEST(file.lexically_normal().generic_string() == "obj/file");
#if defined(_WIN32)
    BOOST_TEST(file.lexically_normal().string() == "obj\\file");
#endif
}

BOOST_AUTO_TEST_CASE(path_base)
{
    using fs::path;
    {
        path base{"."};
        path file{"./file"};

        BOOST_TEST(file.lexically_relative(base).string() == "file");
        BOOST_TEST(relative(file).string() == "file");

        path fileRelated{"file"};
        BOOST_TEST(fileRelated.lexically_relative(base).string() == "");

        BOOST_TEST(relative(fileRelated).string() == "");
    }

    {
        path base{"./component"};
        path file{"./component/src/folder/file"};

        BOOST_TEST(file.lexically_relative(base).generic_string() == "src/folder/file");
        BOOST_TEST(file.lexically_relative(base / "src").generic_string() == "folder/file");
    }

    {
        path base{"./component"};
        path file{"./component/src/folder/file"};

        BOOST_TEST(relative(file, base).generic_string() == "src/folder/file");
    }
}

BOOST_AUTO_TEST_CASE(path_append)
{
    using fs::path;

    path base{"obj/"};
    path file{base};
    file += "./file.ext";
    file += ".o";

    BOOST_TEST(file.string() == "obj/./file.ext.o");
    BOOST_TEST(file.lexically_normal().generic_string() == "obj/file.ext.o");
}
