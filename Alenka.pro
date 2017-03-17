include(build)

TEMPLATE = app console

CONFIG += c++11

QT += widgets websockets

HEADERS += \
	$$files(src/DataModel/*.h) \
	$$files(src/Manager/*.h) \
	$$files(src/SignalProcessor/*.h) \
	$$files(src/Sync/*.h) \
	$$files(src/*.h) \

SOURCES += \
	$$files(src/DataModel/*.cpp) \
	$$files(src/Manager/*.cpp) \
	$$files(src/SignalProcessor/*.cpp) \
	$$files(src/Sync/*.cpp) \
	$$files(src/*.cpp) \

RESOURCES += \
	resources/resources.qrc
