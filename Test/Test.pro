include(../build.txt)

INCLUDEPATH +=../App/src

HEADERS += \
	../App/src/DataFile/datafile.h \
	../App/src/DataFile/gdf2.h \
    gdf2_test.h

SOURCES += \
    test.cpp \
    ../App/src/DataFile/gdf2.cpp

DISTFILES += \
    data/gdf/t00.gdf \
    data/gdf/t01.gdf \
    data/gdf/t00_info.txt \
    data/gdf/t00_values.txt \
    data/gdf/t01_info.txt \
    data/gdf/t01_values.txt



