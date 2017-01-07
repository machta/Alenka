include(build)

TEMPLATE = app console

CONFIG += c++11

QT += widgets

HEADERS += \
	src/DataFile/datafile.h \
	src/DataFile/gdf2.h \
	src/options.h \
	src/canvas.h \
	src/signalviewer.h \
	src/SignalProcessor/signalprocessor.h \
	src/SignalProcessor/signalblock.h \
	src/openglprogram.h \
	src/openglinterface.h \
	src/error.h \
	src/SignalProcessor/gpucache.h \
	src/DataFile/eventtypetable.h \
	src/DataFile/eventtable.h \
	src/DataFile/infotable.h \
	src/signalfilebrowserwindow.h \
	src/DataFile/tracktable.h \
	src/DataFile/montagetable.h \
	src/DataFile/trackcodevalidator.h \
	src/tracklabelbar.h \
	src/myapplication.h \
	src/Manager/manager.h \
	src/Manager/codeeditdialog.h \
	src/Manager/eventmanager.h \
	src/Manager/eventmanagerdelegate.h \
	src/Manager/eventtypemanager.h \
	src/Manager/eventtypemanagerdelegate.h \
	src/Manager/montagemanager.h \
	src/Manager/trackmanager.h \
	src/Manager/trackmanagerdelegate.h \
	src/spikedetanalysis.h \
	src/spikedetsettingsdialog.h \
	src/DataFile/edftmp.h \
	src/SignalProcessor/kernelcache.h

SOURCES += \
	src/DataFile/gdf2.cpp \
	src/options.cpp \
	src/main.cpp \
	src/canvas.cpp \
	src/signalviewer.cpp \
	src/SignalProcessor/signalprocessor.cpp \
	src/SignalProcessor/signalblock.cpp \
	src/openglprogram.cpp \
	src/openglinterface.cpp \
	src/SignalProcessor/gpucache.cpp \
	src/error.cpp \
	src/DataFile/eventtypetable.cpp \
	src/DataFile/eventtable.cpp \
	src/DataFile/datafile.cpp \
	src/DataFile/infotable.cpp \
	src/signalfilebrowserwindow.cpp \
	src/DataFile/tracktable.cpp \
	src/DataFile/montagetable.cpp \
	src/DataFile/trackcodevalidator.cpp \
	src/tracklabelbar.cpp \
	src/myapplication.cpp \
	src/Manager/manager.cpp \
	src/Manager/codeeditdialog.cpp \
	src/Manager/eventmanager.cpp \
	src/Manager/eventmanagerdelegate.cpp \
	src/Manager/eventtypemanager.cpp \
	src/Manager/eventtypemanagerdelegate.cpp \
	src/Manager/montagemanager.cpp \
	src/Manager/trackmanager.cpp \
	src/Manager/trackmanagerdelegate.cpp \
	src/spikedetanalysis.cpp \
	src/spikedetsettingsdialog.cpp \
	src/DataFile/edftmp.cpp \
	src/SignalProcessor/kernelcache.cpp

RESOURCES += \
	resources/resources.qrc
