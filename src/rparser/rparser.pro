TEMPLATE = lib
DEPENDPATH += .
INCLUDEPATH += .
INCLUDEPATH += ../libs/cplusplus
INCLUDEPATH += ../libs/
INCLUDEPATH += ../libs/3rdparty/
INCLUDEPATH += ../libs/3rdparty/cplusplus
INCLUDEPATH += ../plugins/cpptools
include(../../qtcreator.pri)

HEADERS += rparser.h
SOURCES += rparser.cpp
LIBS = -L../../lib/qtcreator -lCPlusPlus -L$$IDE_PLUGIN_PATH
