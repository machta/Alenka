#include <gtest/gtest.h>

#include "../../Alenka-Signal/include/AlenkaSignal/cluster.h"

#include <cmath>
#include <memory>

using namespace std;
using namespace AlenkaSignal;

namespace {

#include "cluster_data.dat"

template <class T1, class T2> int compare(T1 *a, T2 *b, int n) {
  int errorCount = 0;

  for (int i = 0; i < n; ++i) {
    double diff = fabs(a[i] - b[i]);
    double rel = fabs(diff / max<double>(a[i], b[i]));

    if (diff > 0.000001 || rel > 0.0001)
      ++errorCount;
  }

  return errorCount;
}

} // namespace

TEST(cluster_test, test_8kHz) {
  const int channels = 63;
  const int n = 760;

  vector<double> MA, MW;
  MA.reserve(channels * n);
  MW.reserve(channels * n);

  for (int i = 0; i < channels; ++i) {
    for (int j = 0; j < n; ++j) {
      MA.push_back(MA_8kHz[j][i]);
      MW.push_back(MW_8kHz[j][i]);
    }
  }

  auto cluster = make_unique<Cluster>();
  cluster->process(n, channels, MA, MW, false);

  int size = static_cast<int>(cluster->getArea().size());
  EXPECT_EQ(n, size);
  EXPECT_GE(0, compare(cluster->getArea().data(),
                       reinterpret_cast<double *>(area_0_8kHz), n));

  size = static_cast<int>(cluster->getClass().size());
  EXPECT_EQ(n, size);
  EXPECT_GE(0, compare(cluster->getClass().data(),
                       reinterpret_cast<double *>(class_0_8kHz), n));

  size = static_cast<int>(cluster->getWeight().size());
  EXPECT_EQ(n, size);
  EXPECT_GE(0, compare(cluster->getWeight().data(),
                       reinterpret_cast<double *>(weight_0_8kHz), n));

  cluster.release(); // For some reason there is a segfault in the destructor
                     // that doesn't appear when the code is used in Alenka.
                     // Let's ignore it for now. TODO: Find out what the problem
                     // is.
}

TEST(cluster_test, test_8kHz_centering) {
  const int channels = 63;
  const int n = 760;

  vector<double> MA, MW;
  MA.reserve(channels * n);
  MW.reserve(channels * n);

  for (int i = 0; i < channels; ++i) {
    for (int j = 0; j < n; ++j) {
      MA.push_back(MA_8kHz[j][i]);
      MW.push_back(MW_8kHz[j][i]);
    }
  }

  auto cluster = make_unique<Cluster>();
  cluster->process(n, channels, MA, MW, true);

  int size = static_cast<int>(cluster->getArea().size());
  EXPECT_EQ(n, size);
  EXPECT_GE(5, compare(cluster->getArea().data(),
                       reinterpret_cast<double *>(area_1_8kHz), n));

  size = static_cast<int>(cluster->getClass().size());
  EXPECT_EQ(n, size);
  EXPECT_GE(5, compare(cluster->getClass().data(),
                       reinterpret_cast<double *>(class_1_8kHz), n));

  size = static_cast<int>(cluster->getWeight().size());
  EXPECT_EQ(n, size);
  EXPECT_GE(5, compare(cluster->getWeight().data(),
                       reinterpret_cast<double *>(weight_1_8kHz), n));

  cluster.release();
}

TEST(cluster_test, test_sample) {
  const int channels = 19;
  const int n = 224;

  vector<double> MA, MW;
  MA.reserve(channels * n);
  MW.reserve(channels * n);

  for (int i = 0; i < channels; ++i) {
    for (int j = 0; j < n; ++j) {
      MA.push_back(MA_sample[j][i]);
      MW.push_back(MW_sample[j][i]);
    }
  }

  auto cluster = make_unique<Cluster>();
  cluster->process(n, channels, MA, MW, false);

  int size = static_cast<int>(cluster->getArea().size());
  EXPECT_EQ(n, size);
  EXPECT_GE(0, compare(cluster->getArea().data(),
                       reinterpret_cast<double *>(area_0_sample), n));

  size = static_cast<int>(cluster->getClass().size());
  EXPECT_EQ(n, size);
  EXPECT_GE(0, compare(cluster->getClass().data(),
                       reinterpret_cast<double *>(class_0_sample), n));

  size = static_cast<int>(cluster->getWeight().size());
  EXPECT_EQ(n, size);
  EXPECT_GE(0, compare(cluster->getWeight().data(),
                       reinterpret_cast<double *>(weight_0_sample), n));

  cluster.release();
}
