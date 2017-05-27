#include <gtest/gtest.h>

#include <AlenkaSignal/openclcontext.h>
#include <AlenkaSignal/montage.h>

using namespace std;
using namespace AlenkaSignal;

namespace
{

template<class T>
void test()
{
	OpenCLContext context(OPENCL_PLATFORM, OPENCL_DEVICE);

	EXPECT_TRUE(Montage<T>::test("", &context));
	EXPECT_TRUE(Montage<T>::test("out=in(1);", &context));
	EXPECT_TRUE(Montage<T>::test("  out = in(    1     )  ;    ", &context));
	EXPECT_TRUE(Montage<T>::test("out=in(00010);", &context));
	EXPECT_TRUE(Montage<T>::test("int i = 5; out=in(i);", &context));
	EXPECT_TRUE(Montage<T>::test("float sample = in(55); out = 2*sample*2;", &context));
	EXPECT_TRUE(Montage<T>::test("float sample = in(22); out = 2.2*sample*2.2;", &context));
	EXPECT_TRUE(Montage<T>::test("float sample = in(00); out = 2.2*sample*2;", &context));
	EXPECT_TRUE(Montage<T>::test("float sample = in(-1); out = 2*sample*2.2;", &context));
	EXPECT_TRUE(Montage<T>::test("out=in(10 - 2*3);", &context));
	EXPECT_TRUE(Montage<T>::test("in(10 - 2*3);", &context));

	EXPECT_FALSE(Montage<T>::test("fdfsdfssf", &context));
	EXPECT_FALSE(Montage<T>::test("out=in(1)", &context));
	EXPECT_FALSE(Montage<T>::test("out=in();", &context));
	EXPECT_FALSE(Montage<T>::test("out=in(1, 1)", &context));
	EXPECT_FALSE(Montage<T>::test("outt=in(1);", &context));
	EXPECT_FALSE(Montage<T>::test("out=IN(1);", &context));
	EXPECT_FALSE(Montage<T>::test("in(10 3);", &context));
}

template<class T>
void stripComments()
{
	EXPECT_EQ(Montage<T>::stripComments("// bla bla"), "");
	EXPECT_EQ(Montage<T>::stripComments(" // bla bla"), " ");
	EXPECT_EQ(Montage<T>::stripComments("  //*bla bla*/ "), "  ");
	EXPECT_EQ(Montage<T>::stripComments("out = in(1); // bla bla"), "out = in(1); ");

	EXPECT_EQ(Montage<T>::stripComments("/*bla bla*/"), "");
	EXPECT_EQ(Montage<T>::stripComments(" /*bla bla*/ "), "  ");
	EXPECT_EQ(Montage<T>::stripComments("/*bla bla*/ out = in(1);"), " out = in(1);");

	EXPECT_EQ(Montage<T>::stripComments("/*bla \n \n bla bla \n // bbbbbbb \n bla*/"), "");
	EXPECT_EQ(Montage<T>::stripComments(" out = /*bla bladd f ///*/in(1); // bla bla"), " out = in(1); ");
}

} // namespace

TEST(montage_static_test, test_float)
{
	test<float>();
}

TEST(montage_static_test, test_double)
{
	test<double>();
}

TEST(montage_static_test, strip_comments_float)
{
	stripComments<float>();
}

TEST(montage_static_test, strip_comments_double)
{
	stripComments<double>();
}
