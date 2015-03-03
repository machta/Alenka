#include <boost/test/included/unit_test.hpp>

#include <cstdlib>
#include <cstdio>
#include <string>
#include <exception>
#include <vector>

using namespace std;

namespace
{
const int FN = 2;
const char* files[FN] = {"data/gdf/t00", "data/gdf/t01"};

void dataTest(int fi)
{
	string fileName(files[fi]);

	GDF2 gdf((fileName + ".gdf").c_str());
	FILE* file = fopen((fileName + "_values.txt").c_str(), "r");

	int n;
	fscanf(file, "%d", &n);

	vector<double> dataD;
	dataD.insert(dataD.begin(), n, 0);

	vector<float> dataF;
	dataF.insert(dataF.begin(), n, 0);

	gdf.readData(&dataD, 0, n/gdf.getChannelCount() - 1);
	gdf.readData(&dataF, 0, n/gdf.getChannelCount() - 1);

	for (int i = 0; i < n; ++i)
	{
		double value;
		fscanf(file, "%lf", &value);

		if (fabs(value) > 1)
		{
			BOOST_CHECK_CLOSE(dataD[i], value, 0.001);
			BOOST_CHECK_CLOSE((float)dataF[i], value, 0.1);
		}
		else
		{
			BOOST_CHECK_CLOSE(dataD[i], value, 0.01);
			BOOST_CHECK_CLOSE((float)dataF[i], value, 10);
		}
	}

	fclose(file);
}
}

BOOST_AUTO_TEST_CASE(construction)
{
	for (int i = 0; i < FN; ++i)
	{
		string fileName(files[i]);
		fileName += ".gdf";
		BOOST_CHECK_NO_THROW(GDF2 file(fileName.c_str()););
	}
}

BOOST_AUTO_TEST_CASE(exceptions)
{
	BOOST_CHECK_THROW(GDF2 file("blabla"); , runtime_error);
	BOOST_CHECK_THROW(GDF2 file("data/gdf/empty.gdf"); , runtime_error);
	BOOST_CHECK_THROW(GDF2 file("data/gdf/headerOnly.gdf"); , runtime_error);
	BOOST_CHECK_THROW(GDF2 file("data/gdf/badType.gdf"); , runtime_error);
	BOOST_CHECK_THROW(GDF2 file("data/gdf/badFile.gdf"); , runtime_error);

	GDF2* file;

	vector<double> data;
	data.insert(data.begin(), 100000, 0);

	BOOST_REQUIRE_NO_THROW(file = new GDF2("data/gdf/fullHeaderOnly.gdf"));
	BOOST_CHECK_THROW(file->readData(&data, 0, 99); , runtime_error);
	delete file;

	BOOST_REQUIRE_NO_THROW(file = new GDF2("data/gdf/t00.gdf"));
	BOOST_CHECK_THROW(file->readData(&data, 100, 50); , invalid_argument);
	delete file;
}

BOOST_AUTO_TEST_CASE(metaInfo)
{
	for (int i = 0; i < FN; ++i)
	{
		string fileName(files[i]);
		GDF2 gdf((fileName + ".gdf").c_str());
		FILE* file = fopen((fileName + "_info.txt").c_str(), "r");

		double sr;
		int channels, len;
		fscanf(file, "%lf %d %d", &sr, &channels, &len);

		BOOST_CHECK_CLOSE(gdf.getSamplingFrequency(), sr, 0.00001);
		BOOST_CHECK_EQUAL(gdf.getChannelCount(), channels);
		BOOST_CHECK_EQUAL(gdf.getSamplesRecorded(), len);

		fclose(file);
	}
}

BOOST_AUTO_TEST_CASE(data_t00)
{
	dataTest(0);
}

BOOST_AUTO_TEST_CASE(data_t01)
{
	dataTest(1);
}

BOOST_AUTO_TEST_CASE(outOfBounds)
{
	GDF2 file("data/gdf/t00.gdf");
	int n = 100;

	vector<double> a;
	a.insert(a.begin(), (n + 100)*file.getChannelCount(), 0);

	vector<double> b;
	b.insert(b.begin(), n*file.getChannelCount(), 0);

	file.readData(&a, -100, n - 1);
	file.readData(&b, 0, n - 1);

	for (int i = 0; i < 100; ++i)
	{
		for (unsigned int j = 0; j < file.getChannelCount(); ++j)
		{
			BOOST_CHECK_CLOSE(a[(n + 100)*j + i], 0, 0.00001);
		}
	}

	for (int i = 0; i < n; ++i)
	{
		for (unsigned int j = 0; j < file.getChannelCount(); ++j)
		{
			BOOST_CHECK_CLOSE(a[(n + 100)*j + 100 + i], b[n*j + i], 0.00001);
		}
	}

	int last = file.getSamplesRecorded() - 1;
	file.readData(&a, last - n + 1, last + 100);
	file.readData(&b, last - n + 1, last);

	for (int i = 0; i < 100; ++i)
	{
		for (unsigned int j = 0; j < file.getChannelCount(); ++j)
		{
			BOOST_CHECK_CLOSE(a[(n + 100)*j + n + i], 0, 0.00001);
		}
	}

	for (int i = 0; i < n; ++i)
	{
		for (unsigned int j = 0; j < file.getChannelCount(); ++j)
		{
			BOOST_CHECK_CLOSE(a[(n + 100)*j + i], b[n*j + i], 0.00001);
		}
	}
}

