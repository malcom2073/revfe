######################################################################
# Automatically generated by qmake (2.01a) Mon Sep 22 12:29:04 2008
######################################################################

TEMPLATE = lib
TARGET = RevFuelPricesPlugin
DEPENDPATH += .
INCLUDEPATH += . ../../revfe
CONFIG += plugin debug
QT += network
win32:DESTDIR = ../../revfe/plugins/
unix:DESTDIR = ../../revfe/plugins/
CONFIG += embed_manifest_exe
# Input
HEADERS += revfuelprices.h ../../revfe/baseinterface.h ../../revfe/standardmodel.h
SOURCES += revfuelprices.cpp ../../revfe/ipcmessage.cpp ../../revfe/standardmodel.cpp
unix:target.path = /usr/share/revfe/plugins/
unix:INSTALLS += target
