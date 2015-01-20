#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_CASE(simple)
{
	double answerLowpass[] = {-0.105969883127822, 0.0293291419087276, 0.220670858091272, 0.355969883127822, 0.355969883127822, 0.220670858091272, 0.0293291419087275, -0.105969883127822};

	Filter filter(8, 200);

	filter.setLowpass(50);
	filter.setHighpass(-1);
	filter.setNotch(false);
	double* res = filter.computeCoefficients();
	for (int i = 0; i < 8; ++i)
	{
		BOOST_CHECK_CLOSE(res[i], answerLowpass[i], 0.00001);
	}
}

//BOOST_AUTO_TEST_CASE(low200)
//{
//	FILE* file = fopen("C:\\Users\\Martin\\Documents\\MATLAB\\coeff.txt", "w");

//	Filter filter(200, 200);
//	filter.setHighpass(20);
//	filter.setLowpass(80);
//	//filter.setNotch(true);

//	filter.computeCoefficients();
//	filter.printCoefficients(file);

//	fclose(file);
//}
