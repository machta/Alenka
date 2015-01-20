#define BOOST_TEST_ALTERNATIVE_INIT_API
#define BOOST_TEST_NO_MAIN
#define BOOST_TEST_MODULE Test
#include <boost/test/included/unit_test.hpp>

#include "DataFile/gdf2.h"
#include "SignalProcessor/filter.h"
#include "options.h"

#include <clFFT.h>

BOOST_AUTO_TEST_SUITE(DataFile_TS)
	BOOST_AUTO_TEST_SUITE(GDF2_TS)
		#include "gdf2_test.h"
	BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(Filter_TS)
	#include "filter_test.h"
BOOST_AUTO_TEST_SUITE_END()

bool init()
{
	return true;
}

int main(int ac, char** av)
{
	clfftSetupData fftSetup;
	clfftInitSetupData(&fftSetup);
	clfftSetup(&fftSetup);

	Options* options = new Options(1, av);
	PROGRAM_OPTIONS = options;

	int res = boost::unit_test::unit_test_main(init, ac, av);

	clfftTeardown();
	delete options;
	return res;
}
