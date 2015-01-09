#include "DataFile/gdf2.h"

#define BOOST_TEST_MODULE Test
#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(DataFile_TS)
	BOOST_AUTO_TEST_SUITE(GDF2_TS)
		#include "gdf2_test.h"
	BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()






























/*#include "../src/DataFile/gdf2.h"

#include <cstdlib>
#include <cstdio>

using fType = double;

int main()
{
	const int LEN = 20;

	GDF2 gdf("..\\test_data\\sample.gdf");
	//GDF2 gdf("..\\test_data\\sin\\a0.gdf");

	fType* data = new fType[gdf.getChannelCount()*LEN];
	gdf.readData(data, 190, 190 + LEN - 1);
	//gdf.readData(data, 0, 0 + LEN - 1);

	for (int i = 0; i < LEN; ++i)
	{
		for (unsigned int c = 0; c < gdf.getChannelCount()*1 + 0; ++c)
		{
			if (c != 0)
			{
				std::printf(" ");
			}

			std::printf("%10.2f", data[LEN*c + i]);
		}
		std::printf("\n");
	}
}
*/
