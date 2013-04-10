TEMPLATE = lib
DEPENDPATH += .
INCLUDEPATH += .
include(../../qtcreator.pri)

HEADERS += rparser.h
SOURCES += rparser.cpp
message($$IDE_PLUGIN_PATH})
LIBS = -lCPlusPlus -L$$IDE_PLUGIN_PATH
