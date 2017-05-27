cmake_minimum_required(VERSION 3.1)
project(Alenka-Signal)

# Options.
set(CMAKE_CXX_STANDARD 11)
set(BUILD_SHARED_LIBS off CACHE BOOL "")

add_definitions(-DCL_USE_DEPRECATED_OPENCL_1_2_APIS)

if(MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_USE_MATH_DEFINES)
endif()

find_package(OpenCL)
include_directories(${OpenCL_INCLUDE_DIRS})

# Alenka-File
set(Boost_INCLUDE_DIR "")
include_directories(../Alenka-File/include ../libraries/boost_1_63)

# First build all the library dependencies.
set(BUILD_LOADLIBRARIES off CACHE BOOL "")
set(BUILD_EXAMPLES off CACHE BOOL "")
add_subdirectory(../libraries/clFFT/src ${CMAKE_CURRENT_BINARY_DIR}/clFFT)
include_directories(../libraries/clFFT/src/include ${CMAKE_CURRENT_BINARY_DIR}/clFFT/include)

# If you want to use this library, you need to link to these libraries.
set(LIBS_TO_LINK_ALENKA_SIGNAL alenka-signal clFFT ${OpenCL_LIBRARIES})
set(LIBS_TO_LINK_ALENKA_SIGNAL ${LIBS_TO_LINK_ALENKA_SIGNAL} PARENT_SCOPE)

# Alenka-Signal library.
include_directories(include ../libraries/spikedetsrc ../libraries/alglib-3.10.0/src ../libraries/eigen ../libraries/libsamplerate/src ../libraries/SignalResampler/src)

file(GLOB SRC_ALG ../libraries/alglib-3.10.0/src/*.cpp)
file(GLOB SRC_SR ../libraries/libsamplerate/src/*.c)
set(SRC_SD ../libraries/spikedet/src/CSpikeDetector.cpp ../libraries/spikedet/src/CDSP.cpp ../libraries/spikedet/src/CException.cpp)

set(SRC
	include/AlenkaSignal/filter.h
	include/AlenkaSignal/filterprocessor.h
	include/AlenkaSignal/montage.h
	include/AlenkaSignal/montageprocessor.h
	include/AlenkaSignal/openclcontext.h
	include/AlenkaSignal/openclprogram.h
	include/AlenkaSignal/spikedet.h
	src/filter.cpp
	src/filterprocessor.cpp
	src/filtfilt.h
	src/kernels.cl
	src/montage.cpp
	src/montageprocessor.cpp
	src/openclcontext.cpp
	src/openclprogram.cpp
	src/spikedet.cpp
)

add_library(alenka-signal STATIC ${SRC} ${SRC_ALG} ${SRC_SR} ${SRC_SD} ../libraries/SignalResampler/src/resample.cpp)

if(BUILD_TESTS_ALENKA_SIGNAL)
	add_subdirectory(unit-test)
endif()
