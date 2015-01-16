include(../build)

INCLUDEPATH +=../App/src

HEADERS += \
    gdf2_test.h

SOURCES += \
    test.cpp \
    ../App/src/DataFile/gdf2.cpp \
    ../App/src/options.cpp

DISTFILES += \
    data/gdf/t00.gdf \
    data/gdf/t01.gdf \
    data/gdf/t00_info.txt \
    data/gdf/t00_values.txt \
    data/gdf/t01_info.txt \
    data/gdf/t01_values.txt \
    data/gdf/empty.gdf \
    data/gdf/badType.gdf \
    data/gdf/headerOnly.gdf \
    data/gdf/badFile.gdf



