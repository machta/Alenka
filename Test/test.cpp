#include "DataFile/gdf2.h"

#define BOOST_TEST_ALTERNATIVE_INIT_API
#define BOOST_TEST_NO_MAIN
#define BOOST_TEST_MODULE Test
#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(DataFile_TS)
	BOOST_AUTO_TEST_SUITE(GDF2_TS)
		#include "gdf2_test.h"
	BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

#include "options.h"

bool init()
{
    return true;
}

int main(int ac, char** av)
{
    Options* options = new Options(1, av);
    PROGRAM_OPTIONS = options;

    int res = boost::unit_test::unit_test_main(init, ac, av);

    delete options;
    return res;
}
