#ifndef ALENKASIGNAL_CLUSTER_H
#define ALENKASIGNAL_CLUSTER_H

#include <vector>

namespace AlenkaSignal {

class Cluster {
  std::vector<int> clusterClass, clusterArea;
  std::vector<double> clusterWeight;

public:
  Cluster();
  ~Cluster();

  void process(int n, int c, const std::vector<double> &MA,
               const std::vector<double> &MW, bool pca_centering);

  const std::vector<int> &getClass() { return clusterClass; }
  const std::vector<double> &getWeight() { return clusterWeight; }
  const std::vector<int> &getArea() { return clusterArea; }
};

} // namespace AlenkaSignal

#endif // ALENKASIGNAL_CLUSTER_H
