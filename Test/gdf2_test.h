#include <boost/test/included/unit_test.hpp>

const int FN = 2;

const char* files[FN] = {"data/gdf/t00.gdf", "data/gdf/t01.gdf"};

BOOST_AUTO_TEST_CASE(construction)
{
	for (int i = 0; i < FN; ++i)
	{
		GDF2 file(files[i]);
	}
}

BOOST_AUTO_TEST_CASE(metaInfo)
{
}

BOOST_AUTO_TEST_CASE(data)
{
}
