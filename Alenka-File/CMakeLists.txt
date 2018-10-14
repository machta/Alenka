cmake_minimum_required(VERSION 3.1)
project(Alenka-File)

# Options.
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)
set(BUILD_SHARED_LIBS off CACHE BOOL "")

# First build all the library dependencies.
include_directories(../Alenka-File/include/)
include_directories(../Alenka-Signal/include/)
include_directories(../libraries/boost_1_66)

# Platform specific configuration.
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

# Lib EEP -- mostly copied from their cmake file
set(LIBEEP_DIR ../libraries/libeep-3.3.177)
include_directories(${LIBEEP_DIR}/src)

set(LIBEEP_SRC
  ${LIBEEP_DIR}/src/libavr/avr.c
  ${LIBEEP_DIR}/src/libavr/avrcfg.c
  ${LIBEEP_DIR}/src/libcnt/cnt.c
  ${LIBEEP_DIR}/src/libcnt/cntutils.c
  ${LIBEEP_DIR}/src/libcnt/evt.c
  ${LIBEEP_DIR}/src/libcnt/raw3.c
  ${LIBEEP_DIR}/src/libcnt/rej.c
  ${LIBEEP_DIR}/src/libcnt/riff64.c
  ${LIBEEP_DIR}/src/libcnt/riff.c
  ${LIBEEP_DIR}/src/libcnt/seg.c
  ${LIBEEP_DIR}/src/libcnt/trg.c
  ${LIBEEP_DIR}/src/libeep/eepio.c
  ${LIBEEP_DIR}/src/libeep/eepmem.c
  ${LIBEEP_DIR}/src/libeep/eepmisc.c
  ${LIBEEP_DIR}/src/libeep/eepraw.c
  ${LIBEEP_DIR}/src/libeep/val.c
  ${LIBEEP_DIR}/src/libeep/var_string.c
  ${LIBEEP_DIR}/src/v4/eep.c
)

set(LIBEEP_VERSION_MAJOR 3) # Without these defines there is a compile error.
set(LIBEEP_VERSION_MINOR 3)
set(LIBEEP_VERSION_PATCH 178)
add_definitions(-DLIBEEP_VERSION_MAJOR=${LIBEEP_VERSION_MAJOR})
add_definitions(-DLIBEEP_VERSION_MINOR=${LIBEEP_VERSION_MINOR})
add_definitions(-DLIBEEP_VERSION_PATCH=${LIBEEP_VERSION_PATCH})

add_library(eep STATIC ${LIBEEP_SRC})

# If you want to use this library, you need to link to these libraries.
set(LIBS_TO_LINK_ALENKA_FILE alenka-file pugixml matio)
if(UNIX)
    if (ALENKA_USE_BIOSIG)
      set(LIBS_TO_LINK_ALENKA_FILE ${LIBS_TO_LINK_ALENKA_FILE}
        biosig cholmod)
    endif()
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
  include/AlenkaFile/eep.h
  include/AlenkaFile/gdf2.h
  include/AlenkaFile/mat.h
  src/datafile.cpp
  src/datamodel.cpp
  src/edf.cpp
  src/edflib_extended.cpp
  src/edflib_extended.h
  src/eep.cpp
  src/gdf2.cpp
  src/mat.cpp
)
set_source_files_properties(${SRC} PROPERTIES COMPILE_FLAGS ${WARNINGS})

if(ALENKA_USE_BIOSIG)
  set(SRC_BIOSIG src/biosigfile.cpp)
else()
endif()

add_library(alenka-file STATIC ${SRC} ${SRC_BIOSIG} ${SRC_BOOST_S} ${SRC_BOOST_FS})

# TODO: Remove libs from LIBS_TO_LINK... and do it the same way as this.
target_link_libraries(alenka-file eep)
