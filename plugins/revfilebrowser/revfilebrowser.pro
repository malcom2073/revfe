######################################################################
# Automatically generated by qmake (2.01a) Mon Sep 22 12:29:04 2008
######################################################################

TEMPLATE = lib
TARGET = RevFileBrowserPlugin
DEPENDPATH += .
INCLUDEPATH += . ../../revfe
CONFIG += plugin
win32:DESTDIR = ../../revfe/plugins/
unix:DESTDIR = ../../revfe/plugins/
CONFIG += embed_manifest_exe
win32::LIBS += User32.lib
# Input
HEADERS += revfilebrowser.h ../../revfe/baseinterface.h ../../revfe/standardmodel.h
SOURCES += revfilebrowser.cpp ../../revfe/ipcmessage.cpp ../../revfe/standardmodel.cpp
unix:target.path = /usr/share/revfe/plugins/
unix:INSTALLS += target
