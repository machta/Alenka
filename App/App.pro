include(../build)

HEADERS += \
    src/DataFile/datafile.h \
    src/DataFile/gdf2.h \
    src/options.h \
    src/context.h \
    src/program.h \
    src/SignalProcessor/filter.h \
    src/context.h

SOURCES += \
    src/DataFile/gdf2.cpp \
    src/options.cpp \
    src/main.cpp \
    src/context.cpp \
    src/program.cpp \
    src/SignalProcessor/filter.cpp
