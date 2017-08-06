cmake_minimum_required(VERSION 3.1)
project(Cluster)

set(SRC
  cluster.h
  codegen/lib/spike_cluster_cpp/combine_vector_elements.cpp
  codegen/lib/spike_cluster_cpp/eml_rand.cpp
  codegen/lib/spike_cluster_cpp/eml_rand_mt19937ar_stateful.cpp
  codegen/lib/spike_cluster_cpp/eye.cpp
  codegen/lib/spike_cluster_cpp/fileManager.cpp
  codegen/lib/spike_cluster_cpp/fprintf.cpp
  codegen/lib/spike_cluster_cpp/mean.cpp
  codegen/lib/spike_cluster_cpp/mod.cpp
  codegen/lib/spike_cluster_cpp/nullAssignment.cpp
  codegen/lib/spike_cluster_cpp/pca.cpp
  codegen/lib/spike_cluster_cpp/quantile.cpp
  codegen/lib/spike_cluster_cpp/rand.cpp
  codegen/lib/spike_cluster_cpp/randperm.cpp
  codegen/lib/spike_cluster_cpp/repmat.cpp
  codegen/lib/spike_cluster_cpp/rtGetInf.cpp
  codegen/lib/spike_cluster_cpp/rtGetNaN.cpp
  codegen/lib/spike_cluster_cpp/rt_nonfinite.cpp
  codegen/lib/spike_cluster_cpp/sort1.cpp
  codegen/lib/spike_cluster_cpp/sortIdx.cpp
  codegen/lib/spike_cluster_cpp/spike_cluster_cpp.cpp
  codegen/lib/spike_cluster_cpp/spike_cluster_cpp_data.cpp
  codegen/lib/spike_cluster_cpp/spike_cluster_cpp_emxAPI.cpp
  codegen/lib/spike_cluster_cpp/spike_cluster_cpp_emxutil.cpp
  codegen/lib/spike_cluster_cpp/spike_cluster_cpp_initialize.cpp
  codegen/lib/spike_cluster_cpp/spike_cluster_cpp_rtwutil.cpp
  codegen/lib/spike_cluster_cpp/spike_cluster_cpp_terminate.cpp
  codegen/lib/spike_cluster_cpp/sqrt.cpp
  codegen/lib/spike_cluster_cpp/sum.cpp
  codegen/lib/spike_cluster_cpp/unique.cpp
  codegen/lib/spike_cluster_cpp/xaxpy.cpp
  codegen/lib/spike_cluster_cpp/xdotc.cpp
  codegen/lib/spike_cluster_cpp/xnrm2.cpp
  codegen/lib/spike_cluster_cpp/xrot.cpp
  codegen/lib/spike_cluster_cpp/xrotg.cpp
  codegen/lib/spike_cluster_cpp/xscal.cpp
  codegen/lib/spike_cluster_cpp/xswap.cpp)

add_library(cluster STATIC ${SRC})
