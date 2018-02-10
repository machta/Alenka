#include <gtest/gtest.h>

#include "../../Alenka-Signal/include/AlenkaSignal/montage.h"
#include "../../Alenka-Signal/include/AlenkaSignal/openclcontext.h"

using namespace std;
using namespace AlenkaSignal;

TEST(montage_special_test, identity_montage) {
  OpenCLContext context(OPENCL_PLATFORM, OPENCL_DEVICE);

  Montage<float> m("out = in(INDEX);", &context);
  EXPECT_EQ(m.getMontageType(), IdentityMontage);
}
