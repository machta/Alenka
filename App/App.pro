include(../build)

TEMPLATE = app

QT += widgets

HEADERS += \
    src/DataFile/datafile.h \
    src/DataFile/gdf2.h \
    src/options.h \
    src/SignalProcessor/filter.h \
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
    src/eventtypemanager.h \
    src/eventmanager.h \
    src/DataFile/infotable.h \
    src/signalfilebrowserwindow.h \
    src/DataFile/tracktable.h \
    src/DataFile/montagetable.h \
    src/trackmanager.h \
    src/montagemanager.h \
    src/eventmanagerdelegate.h \
    src/eventtypemanagerdelegate.h \
    src/trackmanagerdelegate.h \
    src/codeeditdialog.h \
    src/manager.h \
    src/DataFile/trackcodevalidator.h \
    src/tracklabelbar.h

SOURCES += \
    src/DataFile/gdf2.cpp \
    src/options.cpp \
    src/main.cpp \
    src/SignalProcessor/filter.cpp \
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
    src/DataFile/datafile.cpp \
    src/eventtypemanager.cpp \
    src/eventmanager.cpp \
    src/DataFile/infotable.cpp \
    src/signalfilebrowserwindow.cpp \
    src/DataFile/tracktable.cpp \
    src/DataFile/montagetable.cpp \
    src/trackmanager.cpp \
    src/montagemanager.cpp \
    src/eventmanagerdelegate.cpp \
    src/eventtypemanagerdelegate.cpp \
    src/trackmanagerdelegate.cpp \
    src/codeeditdialog.cpp \
    src/manager.cpp \
    src/DataFile/trackcodevalidator.cpp \
    src/tracklabelbar.cpp

DISTFILES += \
    kernels.cl \
    montageHeader.cl \
    color.frag \
    event.vert \
    signal.vert \
    rectangle.vert
