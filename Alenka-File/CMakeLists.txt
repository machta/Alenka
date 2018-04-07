cmake_minimum_required(VERSION 3.1)
project(Alenka-File)

# Options.
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)
set(BUILD_SHARED_LIBS off CACHE BOOL "")

# Platform specific confuguration.
if(UNIX)
  set(ALENKA_USE_BIOSIG ON)
endif()

# First build all the library dependencies.
include_directories(../libraries/boost_1_66)

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS
    # This stops Microsoft's linker from needing the static library.
    -DBOOST_SYSTEM_NO_LIB -DBOOST_FILESYSTEM_NO_LIB)

  include_directories(../libraries/matio/src ../libraries/matio/visual_studio)

  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    link_directories(../libraries/matio-msvc2015/x64)
  else()
    link_directories(../libraries/matio-msvc2015/x86)
  endif()
endif()

include_directories(../libraries/EDFlib)

set(BUILD_SHARED_LIBS false) # TODO: Check if this is needed.

add_subdirectory(../libraries/pugixml ${CMAKE_CURRENT_BINARY_DIR}/pugixml)
include_directories(../libraries/pugixml/src ../include)

include_directories(../libraries/neuroscan/include)
add_subdirectory(../libraries/neuroscan ${CMAKE_CURRENT_BINARY_DIR}/neuroscan)

# If you want to use this library, you need to link to these libraries.
set(LIBS_TO_LINK_ALENKA_FILE alenka-file pugixml matio neuroscan)
if(UNIX)
  set(LIBS_TO_LINK_ALENKA_FILE ${LIBS_TO_LINK_ALENKA_FILE}
    biosig cholmod)
    #/usr/lib/libbiosig.a)
    #${Alenka_SOURCE_DIR}/libraries/biosig/libbiosig.a)
endif()
set(LIBS_TO_LINK_ALENKA_FILE ${LIBS_TO_LINK_ALENKA_FILE} PARENT_SCOPE)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${DEBUG_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${DEBUG_FLAGS}")

# Alenka-File library.
file(GLOB SRC_BOOST_S ../libraries/boost_1_66/libs/system/src/*.cpp)
file(GLOB SRC_BOOST_FS ../libraries/boost_1_66/libs/filesystem/src/*.cpp)

set(SRC
  include/AlenkaFile/abstractdatamodel.h
  include/AlenkaFile/biosigfile.h
  include/AlenkaFile/datafile.h
  include/AlenkaFile/datamodel.h
  include/AlenkaFile/edf.h
  include/AlenkaFile/gdf2.h
  include/AlenkaFile/mat.h
  src/datafile.cpp
  src/datamodel.cpp
  src/edf.cpp
  src/edflib_extended.cpp
  src/edflib_extended.h
  src/gdf2.cpp
  src/mat.cpp
)
set_source_files_properties(${SRC} PROPERTIES COMPILE_FLAGS ${WARNINGS})

if(ALENKA_USE_BIOSIG)
  set(SRC_BIOSIG src/biosigfile.cpp)
else()
endif()

add_library(alenka-file STATIC ${SRC} ${SRC_BIOSIG} ${SRC_BOOST_S} ${SRC_BOOST_FS})
