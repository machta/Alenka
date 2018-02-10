set(TEST_OPENCL_PLATFORM 0 CACHE STRING "OpenCL platform id used for testing")
set(TEST_OPENCL_DEVICE 0 CACHE STRING "OpenCL device id used for testing")

find_package(OpenMP REQUIRED)
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

add_definitions(
  -DOPENCL_PLATFORM=${TEST_OPENCL_PLATFORM}
  -DOPENCL_DEVICE=${TEST_OPENCL_DEVICE}
  -DTEST_DATA="${CMAKE_CURRENT_SOURCE_DIR}/test-data")

include_directories(../libraries/googletest/googletest/include
  ../libraries/googletest/googletest)

set(SRC
  src/lrucache_test.cpp
  src/file/common.h
  src/file/data_model_test.cpp
  src/file/primary_file_test.cpp
  src/file/save_as_test.cpp
  src/signal/cluster_data.dat
  src/signal/cluster_test.cpp
  src/signal/filter_allpass_test.cpp
  src/signal/filter_design_test.cpp
  src/signal/filter_test.cpp
  src/signal/montage_coordinate_test.cpp
  src/signal/montage_label_test.cpp
  src/signal/montage_special_test.cpp
  src/signal/montage_static_test.cpp
  src/signal/simple_montage_test.cpp
  src/signal/spikedet_test.cpp)

add_executable(unit-test ${SRC}
  ../libraries/googletest/googletest/src/gtest-all.cc
  ../libraries/googletest/googletest/src/gtest_main.cc)

target_link_libraries(unit-test
  ${LIBS_TO_LINK_ALENKA_FILE} ${LIBS_TO_LINK_ALENKA_SIGNAL}
  ${CMAKE_THREAD_LIBS_INIT})

if(WIN32)
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    add_custom_command(TARGET unit-test POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${Alenka_SOURCE_DIR}/libraries/matio-msvc2015/x64/hdf5.dll
      ${Alenka_SOURCE_DIR}/libraries/matio-msvc2015/x64/libmatio.dll
      ${Alenka_SOURCE_DIR}/libraries/matio-msvc2015/x64/szip.dll
      ${Alenka_SOURCE_DIR}/libraries/matio-msvc2015/x64/zlib.dll
      $<TARGET_FILE_DIR:unit-test>)
  else()
    add_custom_command(TARGET unit-test POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${Alenka_SOURCE_DIR}/libraries/matio-msvc2015/x86/hdf5.dll
      ${Alenka_SOURCE_DIR}/libraries/matio-msvc2015/x86/libmatio.dll
      ${Alenka_SOURCE_DIR}/libraries/matio-msvc2015/x86/szip.dll
      ${Alenka_SOURCE_DIR}/libraries/matio-msvc2015/x86/zlib.dll
      $<TARGET_FILE_DIR:unit-test>)
  endif()
endif()

# TODO: Split this file into subprojects the same way Alenka is, so that some of
# the tests can be built and run independently.
