include(../build)

INCLUDEPATH +=../App/src

HEADERS += \
    src/filter_test.h \
    src/gdf2_test.h \
    ../App/src/DataFile/eventtable.h \
    ../App/src/DataFile/eventtypetable.h \
    ../App/src/DataFile/montagetable.h

SOURCES += \
	src/test.cpp\
	../App/src/DataFile/gdf2.cpp \
	../App/src/SignalProcessor/filter.cpp \
	../App/src/openclcontext.cpp \
	../App/src/openclprogram.cpp \
	../App/src/options.cpp \
	../App/src/error.cpp \
	../App/src/DataFile/datafile.cpp \
	../App/src/DataFile/eventtable.cpp \
	../App/src/DataFile/eventtypetable.cpp \
	../App/src/DataFile/montagetable.cpp

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



