######################################################################
# Automatically generated by qmake (2.01a) Mon Sep 22 12:29:04 2008
######################################################################

TEMPLATE = lib
TARGET = RevSocketInterfacePlugin
DEPENDPATH += .
INCLUDEPATH += . ../../revfe
CONFIG += plugin debug
win32:DESTDIR = ../../revfe/plugins/
unix:DESTDIR = ../../revfe/plugins/
QT += network
CONFIG += embed_manifest_exe
# Input
HEADERS += revsocketinterface.h ../../revfe/baseinterface.h
SOURCES += revsocketinterface.cpp ../../revfe/ipcmessage.cpp
unix:target.path = /usr/share/revfe/plugins/
unix:INSTALLS += target
