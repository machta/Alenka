include(build)

TEMPLATE = app console

CONFIG += c++11

QT += widgets websockets

HEADERS += \
	src/DataFile/datafile.h \
	src/DataFile/gdf2.h \
	src/DataFile/eventtypetable.h \
	src/DataFile/eventtable.h \
	src/DataFile/infotable.h \
	src/DataFile/tracktable.h \
	src/DataFile/montagetable.h \
	src/DataFile/trackcodevalidator.h \
	src/DataFile/edftmp.h \
	src/DataFile/vitnessdatamodel.h \
	src/Manager/manager.h \
	src/Manager/codeeditdialog.h \
	src/Manager/eventmanager.h \
	src/Manager/eventmanagerdelegate.h \
	src/Manager/eventtypemanager.h \
	src/Manager/eventtypemanagerdelegate.h \
	src/Manager/montagemanager.h \
	src/Manager/trackmanager.h \
	src/Manager/trackmanagerdelegate.h \
	src/Manager/tablemodel.h \
	src/Manager/eventtypetablemodel.h \
	src/Manager/eventtablemodel.h \
	src/Manager/tracktablemodel.h \
	src/Manager/montagetablemodel.h \
	src/SignalProcessor/signalprocessor.h \
	src/SignalProcessor/signalblock.h \
	src/SignalProcessor/gpucache.h \
	src/signalfilebrowserwindow.h \
	src/SignalProcessor/kernelcache.h \
	src/Sync/syncserver.h \
	src/Sync/syncclient.h \
	src/Sync/syncdialog.h \
	src/options.h \
	src/canvas.h \
	src/signalviewer.h \
	src/openglprogram.h \
	src/openglinterface.h \
	src/error.h \
	src/tracklabelbar.h \
	src/myapplication.h \
	src/spikedetanalysis.h \
	src/spikedetsettingsdialog.h \

SOURCES += \
	src/DataFile/gdf2.cpp \
	src/DataFile/eventtypetable.cpp \
	src/DataFile/eventtable.cpp \
	src/DataFile/datafile.cpp \
	src/DataFile/infotable.cpp \
	src/DataFile/tracktable.cpp \
	src/DataFile/montagetable.cpp \
	src/DataFile/trackcodevalidator.cpp \
	src/DataFile/edftmp.cpp \
	src/DataFile/vitnessdatamodel.cpp \
	src/Manager/manager.cpp \
	src/Manager/codeeditdialog.cpp \
	src/Manager/eventmanager.cpp \
	src/Manager/eventmanagerdelegate.cpp \
	src/Manager/eventtypemanager.cpp \
	src/Manager/eventtypemanagerdelegate.cpp \
	src/Manager/montagemanager.cpp \
	src/Manager/trackmanager.cpp \
	src/Manager/trackmanagerdelegate.cpp \
	src/Manager/tablemodel.cpp \
	src/Manager/eventtypetablemodel.cpp \
	src/Manager/eventtablemodel.cpp \
	src/Manager/tracktablemodel.cpp \
	src/Manager/montagetablemodel.cpp \
	src/SignalProcessor/signalprocessor.cpp \
	src/SignalProcessor/signalblock.cpp \
	src/SignalProcessor/gpucache.cpp \
	src/SignalProcessor/kernelcache.cpp \
	src/Sync/syncserver.cpp \
	src/Sync/syncclient.cpp \
	src/Sync/syncdialog.cpp \
	src/options.cpp \
	src/main.cpp \
	src/canvas.cpp \
	src/signalviewer.cpp \
	src/openglprogram.cpp \
	src/openglinterface.cpp \
	src/error.cpp \
	src/signalfilebrowserwindow.cpp \
	src/tracklabelbar.cpp \
	src/myapplication.cpp \
	src/spikedetanalysis.cpp \
	src/spikedetsettingsdialog.cpp \

RESOURCES += \
	resources/resources.qrc
