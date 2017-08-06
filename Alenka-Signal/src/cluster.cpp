#include "../include/AlenkaSignal/cluster.h"

#include "../../libraries/cluster/cluster.h"

#include <assert.h>

using namespace std;
// using namespace AlenkaSignal;

namespace {

emxArray_real_T *initMatrix(int m, int n) {
  int sizeArray[2] = {m, n};
  return emxCreateND_real_T(2, *(int(*)[2]) & sizeArray[0]);
}

} // namespace

namespace AlenkaSignal {

Cluster::Cluster() { spike_cluster_cpp_initialize(); }

Cluster::~Cluster() { spike_cluster_cpp_terminate(); }

void Cluster::process(int n, int c, const std::vector<double> &MA,
                      const vector<double> &MW, bool pca_centering) {
  assert(static_cast<int>(MA.size()) == n * c);
  assert(static_cast<int>(MW.size()) == n * c);

  emxArray_real_T *MA_mat = initMatrix(n, c);
  emxArray_real_T *MW_mat = initMatrix(n, c);
  struct0_T cluster;
  emxInit_struct0_T(&cluster);

  for (int i = 0; i < n * c; ++i) {
    MA_mat->data[i] = MA[i];
    MW_mat->data[i] = MW[i];
  }

  spike_cluster_cpp(MA_mat, MW_mat, pca_centering ? 1.0 : 0.0, &cluster);
  clusterClass.resize(n);
  clusterWeight.resize(n);
  clusterArea.resize(n);

  for (int i = 0; i < n; ++i) {
    clusterClass[i] = static_cast<int>(cluster.b_class->data[i]);
    clusterWeight[i] = cluster.weight->data[i];
    clusterArea[i] = static_cast<int>(cluster.area->data[i]);
  }

  emxDestroyArray_real_T(MW_mat);
  emxDestroyArray_real_T(MA_mat);
  emxDestroy_struct0_T(cluster);
}

} // namespace AlenkaSignal
