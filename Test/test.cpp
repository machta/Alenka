#include "../src/DataFile/gdf2.h"

#include <cstdlib>
#include <cstdio>

int main()
{
	const int LEN = 50;

	GDF2 gdf("..\\test_data\\sample.gdf");

	double* data = new double[gdf.getChannelCount()*LEN];
	gdf.readData(data, 180, 180 + LEN - 1);

	for (int i = 0; i < LEN; ++i)
	{
		for (int c = 0; c < gdf.getChannelCount()*0 + 1; ++c)
		{
			if (c != 0)
			{
				std::printf(" ");
			}

			std::printf("%10.2lf", data[LEN*c + i]);
		}
		std::printf("\n");
	}
}
