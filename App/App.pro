include(../build)

TEMPLATE = app

QT += widgets

HEADERS += \
    src/DataFile/datafile.h \
    src/DataFile/gdf2.h \
    src/options.h \
    src/SignalProcessor/filter.h \
    src/testwindow.h \
    src/canvas.h \
    src/openclcontext.h \
    src/openclprogram.h

SOURCES += \
    src/DataFile/gdf2.cpp \
    src/options.cpp \
    src/main.cpp \
    src/SignalProcessor/filter.cpp \
    src/testwindow.cpp \
    src/canvas.cpp \
    src/openclcontext.cpp \
    src/openclprogram.cpp

FORMS += \
    src/testwindow.ui
