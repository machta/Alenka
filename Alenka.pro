include(build)

TEMPLATE = app console

CONFIG += c++11

QT += widgets websockets charts qml quick quickwidgets# core

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
	resources/icons/icons.qrc \
	resources/gpu-code/gpu-code.qrc \

# Tests.
INCLUDEPATH += unit-test/googletest/googletest/include unit-test/googletest/googletest
SOURCES += $$files(unit-test/*.cpp)
