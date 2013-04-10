TEMPLATE = lib
DEPENDPATH += .
INCLUDEPATH += . ../libs/cplusplus
INCLUDEPATH += . ../libs/
INCLUDEPATH += . ../libs/3rdparty/
INCLUDEPATH += . ../libs/3rdparty/cplusplus
include(../../qtcreator.pri)

HEADERS += rparser.h
SOURCES += rparser.cpp
message($$IDE_PLUGIN_PATH})
LIBS = -lCPlusPlus -L$$IDE_PLUGIN_PATH
