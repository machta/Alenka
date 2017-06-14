#include <gtest/gtest.h>

#include "../../Alenka-Signal/include/AlenkaSignal/montage.h"
#include "../../Alenka-Signal/include/AlenkaSignal/openclcontext.h"

using namespace std;
using namespace AlenkaSignal;

TEST(montage_label_test, one_parameter) {
  vector<string> labels{"L0", "L1"};
  OpenCLContext context(OPENCL_PLATFORM, OPENCL_DEVICE);

  Montage<float> m0("out = in(\"L0\");", &context, "", labels);
  EXPECT_TRUE(m0.isCopyMontage());
  EXPECT_EQ(m0.copyMontageIndex(), 0);

  Montage<float> m1("out = in(\"L1\");", &context, "", labels);
  EXPECT_TRUE(m1.isCopyMontage());
  EXPECT_EQ(m1.copyMontageIndex(), 1);

  Montage<float> m01("out = in(\"L0\") + x(\"L1\");", &context, "", labels);
  Montage<float> m01_index("out = in(0) + x(1);", &context, "", labels);
  EXPECT_FALSE(m01.isCopyMontage());
  EXPECT_EQ(m01.getSource(), m01_index.getSource());

  Montage<float> mBad("out = in(\"bla bla\");", &context, "", labels);
  EXPECT_TRUE(!mBad.isCopyMontage());
}

TEST(montage_label_test, double_parameter) {
  vector<string> labels{"L0", "L1"};
  OpenCLContext context(OPENCL_PLATFORM, OPENCL_DEVICE);

  Montage<float> m("out = in(\"L0\")*dist(\"L0\", \"L1\");", &context, "",
                   labels);
  Montage<float> m_index("out = in(0)*dist(0, 1);", &context, "", labels);
  EXPECT_FALSE(m.isCopyMontage());
  EXPECT_EQ(m.getSource(), m_index.getSource());
}
