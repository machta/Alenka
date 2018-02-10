#include <gtest/gtest.h>

#include "../../Alenka-Signal/include/AlenkaSignal/montage.h"
#include "../../Alenka-Signal/include/AlenkaSignal/openclcontext.h"

using namespace std;
using namespace AlenkaSignal;

TEST(montage_label_test, one_parameter) {
  vector<string> labels{"L0", "L1"};
  OpenCLContext context(OPENCL_PLATFORM, OPENCL_DEVICE);

  Montage<float> m0("out = in(\"L0\");", &context, "", labels);
  EXPECT_EQ(m0.getMontageType(), CopyMontage);
  EXPECT_EQ(m0.copyMontageIndex(), 0);

  Montage<float> m1("out = in(\"L1\");", &context, "", labels);
  EXPECT_EQ(m1.getMontageType(), CopyMontage);
  EXPECT_EQ(m1.copyMontageIndex(), 1);

  Montage<float> m01("out = in(\"L0\") + x(\"L1\");", &context, "", labels);
  Montage<float> m01_index("out = in(0) + x(1);", &context, "", labels);
  EXPECT_EQ(m01.getMontageType(), NormalMontage);
  EXPECT_EQ(m01.getSource(), m01_index.getSource());

  Montage<float> mBad("out = in(\"bla bla\");", &context, "", labels);
  EXPECT_EQ(mBad.getMontageType(), NormalMontage);
}

TEST(montage_label_test, double_parameter) {
  vector<string> labels{"L0", "L1"};
  OpenCLContext context(OPENCL_PLATFORM, OPENCL_DEVICE);

  Montage<float> m("out = in(\"L0\")*dist(\"L0\", \"L1\");", &context, "",
                   labels);
  Montage<float> m_index("out = in(0)*dist(0, 1);", &context, "", labels);
  EXPECT_EQ(m.getMontageType(), NormalMontage);
  EXPECT_EQ(m.getSource(), m_index.getSource());
}
