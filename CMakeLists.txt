cmake_minimum_required(VERSION 3.1)
project(Alenka)

# Options.
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

option(ALENKA_STATIC_LINK
"Link some libraries statically to make a portable binary.
Use this to make a package that works for both Ubuntu 14 and 15." OFF)
option(ALENKA_BUILD_TESTS "Whether to build unit-tests for Alenka." OFF)
option(ALENKA_COVERAGE "Turns on lcov." OFF)
option(ALENKA_USE_BIOSIG "Include biosig DataFile (experimental, Linux only)." OFF)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(DEBUG true)
endif()

if (ALENKA_USE_BIOSIG)
  add_definitions(-DUNIX_BUILD -DUSE_BIOSIG)
endif()

# Platform specific confuguration.
if(UNIX)
  add_definitions(-DUNIX_BUILD)

  if(ALENKA_COVERAGE)
    set(DEBUG_FLAGS "--coverage")
  endif()

  if(DEBUG)
    #add_definitions(-D_GLIBCXX_DEBUG)
  endif()
elseif(WIN32)
  # Parallel compilation mode for cl.exe.
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")

  add_definitions(-DWIN_BUILD -D_USE_MATH_DEFINES -D_CRT_SECURE_NO_WARNINGS
    # These stops Microsoft's linker from needing the static library.
    -DBOOST_PROGRAM_OPTIONS_NO_LIB
    -DBOOST_SYSTEM_NO_LIB
    -DBOOST_FILESYSTEM_NO_LIB)

  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    link_directories(${Alenka_SOURCE_DIR}/libraries/matio-msvc2015/x64)
  else()
    link_directories(${Alenka_SOURCE_DIR}/libraries/matio-msvc2015/x86)
  endif()
elseif(APPLE)
  add_definitions(-DMAC_BUILD)
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(WARNINGS "-Wall -pedantic -Wextra")# -Wconversion")
  # Supress some gcc warnings.
  set(NO_WARNINGS "-Wno-ignored-attributes")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR
  "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
  set(WARNINGS "-Wall -pedantic -Wextra")# -Wconversion")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  #set(WARNINGS "-W4") # This can be used to get some additional warnings.
  set(WARNINGS " ") # The space must be there, otherwise there is a cmake error.
endif()

# Find packages.
find_package(Threads)
find_package(OpenCL)

find_package(Qt5Widgets)
find_package(Qt5WebSockets) # This is for Syncronization
find_package(Qt5Charts) # This is needed for FilterVisualizer
find_package(Qt5Qml) # These three are for Elko
find_package(Qt5Quick)
find_package(Qt5QuickWidgets)
find_package(Qt5Multimedia) # These two are needed by VideoPlayer
find_package(Qt5MultimediaWidgets)

# Dependencies.
add_subdirectory(Alenka-File)
add_subdirectory(Alenka-Signal)

include_directories(Alenka-File/include/)
include_directories(Alenka-Signal/include/)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(OpenMP REQUIRED)
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS} ${DEBUG_FLAGS}")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS} ${DEBUG_FLAGS}")

# Alenka executable.
add_definitions(-DCL_USE_DEPRECATED_OPENCL_1_2_APIS
  -DCL_USE_DEPRECATED_OPENCL_2_0_APIS) # To silence some silly warnings.

include_directories(libraries/boost_1_66 libraries/pugixml/src libraries/eigen
  ${OpenCL_INCLUDE_DIR} include)

file(GLOB SRC_BOOST_PO libraries/boost_1_66/libs/program_options/src/*.cpp)

set(RESOURCES
  libraries/ElkoAlenka/resources/resources.qrc
  resources/gpu-code/gpu-code.qrc
  resources/icons/icons.qrc)

set(SRC
  include/detailedexception.h
  include/elidedlabel.h
  include/helplink.h
  include/localeoverride.h
  src/DataModel/infotable.cpp
  src/DataModel/infotable.h
  src/DataModel/kernelcache.cpp
  src/DataModel/kernelcache.h
  src/DataModel/opendatafile.cpp
  src/DataModel/opendatafile.h
  src/DataModel/trackcodevalidator.cpp
  src/DataModel/trackcodevalidator.h
  src/DataModel/undocommandfactory.cpp
  src/DataModel/undocommandfactory.h
  src/DataModel/vitnessdatamodel.cpp
  src/DataModel/vitnessdatamodel.h
  src/Manager/codeeditdialog.cpp
  src/Manager/codeeditdialog.h
  src/Manager/dummywidget.h
  src/Manager/eventmanager.cpp
  src/Manager/eventmanager.h
  src/Manager/eventtablemodel.cpp
  src/Manager/eventtablemodel.h
  src/Manager/eventtypemanager.cpp
  src/Manager/eventtypemanager.h
  src/Manager/eventtypetablemodel.cpp
  src/Manager/eventtypetablemodel.h
  src/Manager/filtermanager.cpp
  src/Manager/filtermanager.h
  src/Manager/filtervisualizer.cpp
  src/Manager/filtervisualizer.h
  src/Manager/manager.cpp
  src/Manager/manager.h
  src/Manager/montagemanager.cpp
  src/Manager/montagemanager.h
  src/Manager/montagetablemodel.cpp
  src/Manager/montagetablemodel.h
  src/Manager/sortproxymodel.cpp
  src/Manager/sortproxymodel.h
  src/Manager/tablemodel.cpp
  src/Manager/tablemodel.h
  src/Manager/trackmanager.cpp
  src/Manager/trackmanager.h
  src/Manager/tracktablemodel.cpp
  src/Manager/tracktablemodel.h
  src/Manager/videoplayer.cpp
  src/Manager/videoplayer.h
  src/SignalProcessor/analysis.h
  src/SignalProcessor/automaticmontage.h
  src/SignalProcessor/bipolarmontage.cpp
  src/SignalProcessor/bipolarmontage.h
  src/SignalProcessor/defaultmontage.cpp
  src/SignalProcessor/defaultmontage.h
  src/SignalProcessor/clusteranalysis.cpp
  src/SignalProcessor/clusteranalysis.h
  src/SignalProcessor/lrucache.h
  src/SignalProcessor/modifiedspikedetanalysis.h
  src/SignalProcessor/signalprocessor.cpp
  src/SignalProcessor/signalprocessor.h
  src/SignalProcessor/spikedetanalysis.cpp
  src/SignalProcessor/spikedetanalysis.h
  src/Sync/syncclient.cpp
  src/Sync/syncclient.h
  src/Sync/syncdialog.cpp
  src/Sync/syncdialog.h
  src/Sync/syncserver.cpp
  src/Sync/syncserver.h
  src/canvas.cpp
  src/canvas.h
  src/error.cpp
  src/error.h
  src/filetype.cpp
  src/filetype.h
  src/main.cpp
  src/montagetemplatedialog.cpp
  src/montagetemplatedialog.h
  src/myapplication.cpp
  src/myapplication.h
  src/openglinterface.cpp
  src/openglinterface.h
  src/openglprogram.cpp
  src/openglprogram.h
  src/options.cpp
  src/options.h
  src/signalfilebrowserwindow.cpp
  src/signalfilebrowserwindow.h
  src/signalviewer.cpp
  src/signalviewer.h
  src/spikedetsettingsdialog.cpp
  src/spikedetsettingsdialog.h
  src/tracklabelbar.cpp
  src/tracklabelbar.h
)
set_source_files_properties(${SRC} PROPERTIES COMPILE_FLAGS
  "${WARNINGS} ${NO_WARNINGS}")

add_executable(Alenka ${SRC} ${RESOURCES} ${SRC_BOOST_PO}
  libraries/spikedet/src/CResultsModel.cpp)

target_link_libraries(Alenka
  alenka-signal clFFT cluster
  alenka-file pugixml
  # matio is excluded here on purpose, because I want to link it differently on
  # different platforms.
  ${CMAKE_THREAD_LIBS_INIT} ${OpenCL_LIBRARIES}
  Qt5::Widgets
  Qt5::WebSockets
  Qt5::Charts
  Qt5::Qml
  Qt5::Quick
  Qt5::QuickWidgets
  Qt5::Multimedia
  Qt5::MultimediaWidgets
)

if(UNIX AND NOT APPLE)
  target_link_libraries(Alenka dl GL)

  if (ALENKA_USE_BIOSIG)
    target_link_libraries(Alenka biosig cholmod)
  endif()

  if(ALENKA_STATIC_LINK)
    target_link_libraries(Alenka
      /usr/lib/gcc/x86_64-linux-gnu/5/libstdc++.a
      /usr/lib/gcc/x86_64-linux-gnu/5/libgomp.a
      /usr/lib/x86_64-linux-gnu/libmatio.a
      /usr/lib/x86_64-linux-gnu/libhdf5_serial.a
      /usr/lib/x86_64-linux-gnu/libsz.a
      /usr/lib/x86_64-linux-gnu/libaec.a
      /usr/lib/x86_64-linux-gnu/libz.a)
  else()
    target_link_libraries(Alenka matio)
  endif()
elseif(WIN32)
  include_directories(libraries/matio/src libraries/matio/visual_studio)
  target_link_libraries(Alenka opengl32)

  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    target_link_libraries(Alenka
      ${Alenka_SOURCE_DIR}/libraries/matio-msvc2015/x64/matio.lib)

    add_custom_command(TARGET Alenka POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${Alenka_SOURCE_DIR}/libraries/matio-msvc2015/x64/hdf5.dll
      ${Alenka_SOURCE_DIR}/libraries/matio-msvc2015/x64/libmatio.dll
      ${Alenka_SOURCE_DIR}/libraries/matio-msvc2015/x64/szip.dll
      ${Alenka_SOURCE_DIR}/libraries/matio-msvc2015/x64/zlib.dll
      $<TARGET_FILE_DIR:Alenka>)
  else()
    target_link_libraries(Alenka
      ${Alenka_SOURCE_DIR}/libraries/matio-msvc2015/x86/matio.lib)

    add_custom_command(TARGET Alenka POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${Alenka_SOURCE_DIR}/libraries/matio-msvc2015/x86/hdf5.dll
      ${Alenka_SOURCE_DIR}/libraries/matio-msvc2015/x86/libmatio.dll
      ${Alenka_SOURCE_DIR}/libraries/matio-msvc2015/x86/szip.dll
      ${Alenka_SOURCE_DIR}/libraries/matio-msvc2015/x86/zlib.dll
      $<TARGET_FILE_DIR:Alenka>)
  endif()
elseif(APPLE)
  target_link_libraries(Alenka matio)
endif()

add_custom_command(TARGET Alenka POST_BUILD COMMAND ${CMAKE_COMMAND}
  -E make_directory $<TARGET_FILE_DIR:Alenka>/log
  $<TARGET_FILE_DIR:Alenka>/montageTemplates)

add_custom_command(TARGET Alenka POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${Alenka_SOURCE_DIR}/misc/deploy/montageTemplates
  $<TARGET_FILE_DIR:Alenka>/montageTemplates)

add_custom_command(TARGET Alenka POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  ${Alenka_SOURCE_DIR}/misc/deploy/montageHeader.cl
  $<TARGET_FILE_DIR:Alenka>)

# Tests.
set(CMAKE_AUTOMOC OFF)
set(CMAKE_AUTORCC OFF)

if(ALENKA_BUILD_TESTS)
  add_subdirectory(unit-test)
endif()
