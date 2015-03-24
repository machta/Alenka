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
    src/openclprogram.h \
    src/signalviewer.h \
    src/SignalProcessor/signalprocessor.h \
    src/SignalProcessor/signalblock.h \
    src/openglprogram.h \
    src/openglinterface.h \
    src/error.h \
    src/SignalProcessor/filterprocessor.h \
    src/SignalProcessor/montageprocessor.h \
    src/SignalProcessor/montage.h \
    src/SignalProcessor/gpucache.h \
    src/DataFile/eventtypetable.h \
    src/DataFile/eventtable.h \
    src/DataFile/montagetable.h \
    src/eventtypemanager.h

SOURCES += \
    src/DataFile/gdf2.cpp \
    src/options.cpp \
    src/main.cpp \
    src/SignalProcessor/filter.cpp \
    src/testwindow.cpp \
    src/canvas.cpp \
    src/openclcontext.cpp \
    src/openclprogram.cpp \
    src/signalviewer.cpp \
    src/SignalProcessor/signalprocessor.cpp \
    src/SignalProcessor/signalblock.cpp \
    src/openglprogram.cpp \
    src/openglinterface.cpp \
    src/SignalProcessor/filterprocessor.cpp \
    src/SignalProcessor/montageprocessor.cpp \
    src/SignalProcessor/montage.cpp \
    src/SignalProcessor/gpucache.cpp \
    src/error.cpp \
    src/DataFile/eventtypetable.cpp \
    src/DataFile/eventtable.cpp \
    src/DataFile/montagetable.cpp \
    src/DataFile/datafile.cpp \
    src/eventtypemanager.cpp

FORMS += \
    src/testwindow.ui \
    src/eventtypemanager.ui

DISTFILES += \
    shader.vert \
    shader.frag \
    kernels.cl \
    montageHeader.cl
