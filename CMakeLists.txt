cmake_minimum_required(VERSION 3.1)
project(Alenka)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(DEBUG true)
endif()

# Options.
set(CMAKE_CXX_STANDARD 11)

if(UNIX)
	add_definitions(-DUNIX_BUILD)

	if(DEBUG)
		add_definitions(-D_GLIBCXX_DEBUG)
	endif()
elseif(WIN32)
	add_definitions(-DWIN_BUILD -D_USE_MATH_DEFINES -D_CRT_SECURE_NO_WARNINGS
		# This stops Microsoft's linker from needing the static library.
		-DBOOST_PROGRAM_OPTIONS_NO_LIB)
elseif(APPLE)
endif()

# Find packages.
find_package (Threads)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5Widgets)
find_package(Qt5WebSockets)
find_package(Qt5Charts)
find_package(Qt5Qml)
find_package(Qt5Quick)
find_package(Qt5QuickWidgets)

# Dependencies.
add_subdirectory(Alenka-File)
add_subdirectory(Alenka-Signal)

# Tests.
if(DEBUG)
	add_definitions(-DTESTS)
	include_directories(unit-test/googletest/googletest/include unit-test/googletest/googletest)
	file(GLOB SRC_TESTS unit-test/*.cpp)
endif()

# Alenka.
add_definitions(-DCL_USE_DEPRECATED_OPENCL_1_2_APIS)

include_directories(Alenka-File/include Alenka-Signal/include
	${Alenka-File_SOURCE_DIR}/boost ${Alenka-File_SOURCE_DIR}/pugixml/src
	${Alenka-Signal_SOURCE_DIR}/eigen)

file(GLOB_RECURSE SRC src/*.cpp src/*.h) # TODO: Specify each source file explicitly.
file(GLOB SRC_BOOST_PO Alenka-File/boost/libs/program_options/src/*.cpp)
set(RESOURCE resources/icons/icons.qrc resources/gpu-code/gpu-code.qrc elko/resources/resources.qrc)

add_executable(Alenka
	${SRC} ${SRC_BOOST_PO} ${RESOURCE} ${SRC_TESTS}
	Alenka-Signal/spikedet/src/CResultsModel.cpp)

target_link_libraries(Alenka OpenCL alenka-signal alenka-file clFFT GDF pugixml
	${CMAKE_THREAD_LIBS_INIT}
	Qt5::Widgets Qt5::WebSockets Qt5::Charts Qt5::Qml Qt5::Quick Qt5::QuickWidgets)

if(UNIX)
	target_link_libraries(Alenka dl GL
		# Link these libraries staticaly so that the binary can be used on Ubuntu 14 as well.
		/usr/lib/gcc/x86_64-linux-gnu/5/libstdc++.a
		/usr/lib/gcc/x86_64-linux-gnu/5/libgomp.a
		/usr/lib/x86_64-linux-gnu/libmatio.a
		/usr/lib/x86_64-linux-gnu/libhdf5_serial.a
		/usr/lib/x86_64-linux-gnu/libsz.a
		/usr/lib/x86_64-linux-gnu/libaec.a
		/usr/lib/x86_64-linux-gnu/libz.a)
elseif(WIN32)
	target_link_libraries(Alenka opengl32 matio)
	include_directories(Alenka-File/matio/src Alenka-File/matio/visual_studio)

	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		add_custom_command(TARGET Alenka POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"Alenka-File/matio/matio-msvc2015/x64/hdf5.dll"
			"Alenka-File/matio/matio-msvc2015/x64/libmatio.dll"
			"Alenka-File/matio/matio-msvc2015/x64/szip.dll"
			"Alenka-File/matio/matio-msvc2015/x64/zlib.dll"
			$<TARGET_FILE_DIR:Alenka>)
	else()
		add_custom_command(TARGET Alenka POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"Alenka-File/matio/matio-msvc2015/x64/hdf5.dll"
			"Alenka-File/matio/matio-msvc2015/x64/libmatio.dll"
			"Alenka-File/matio/matio-msvc2015/x64/szip.dll"
			"Alenka-File/matio/matio-msvc2015/x64/zlib.dll"
			$<TARGET_FILE_DIR:Alenka>)
	endif()
elseif(APPLE)
endif()
