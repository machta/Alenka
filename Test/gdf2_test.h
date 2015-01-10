#include <boost/test/included/unit_test.hpp>

#include "DataFile/gdf2.h"

#include <cstdlib>
#include <cstdio>
#include <string>
#include <exception>

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

        double* dataD = new double[n];
        float* dataF = new float[n];
        gdf.readData(dataD, 0, n/gdf.getChannelCount() - 1);
        gdf.readData(dataF, 0, n/gdf.getChannelCount() - 1);

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

        delete[] dataD;
        delete[] dataF;
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
    BOOST_CHECK_THROW(GDF2 file("blabla");, runtime_error);
    BOOST_CHECK_THROW(GDF2 file("data/gdf/empty.gdf");, runtime_error);
    BOOST_CHECK_THROW(GDF2 file("data/gdf/headerOnly.gdf");, runtime_error);
    BOOST_CHECK_THROW(GDF2 file("data/gdf/badType.gdf");, runtime_error);
    BOOST_CHECK_THROW(GDF2 file("data/gdf/badFile.gdf");, runtime_error);

    GDF2* file;
    BOOST_REQUIRE_NO_THROW(file = new GDF2("data/gdf/fullHeaderOnly.gdf"));
    double* data = new double[100*file->getChannelCount()];
    BOOST_CHECK_THROW(file->readData(data, 0, 99);, runtime_error);
    delete file;
    delete[] data;
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
