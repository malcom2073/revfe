# #####################################################################
# Automatically generated by qmake (2.01a) Mon Sep 22 12:29:04 2008
# #####################################################################
TEMPLATE = lib
TARGET = AppLauncherPlugin
DEPENDPATH += .
INCLUDEPATH += . \
    ../../revfe \
    ../medialibrary
CONFIG += plugin debug
win32:DESTDIR = ../../revfe/plugins/
unix:DESTDIR = ../../revfe/plugins/
CONFIG += embed_manifest_exe

# Input
HEADERS += applauncher.h \
    ../../revfe/baseinterface.h
SOURCES += applauncher.cpp \
    ../../revfe/ipcmessage.cpp
unix:target.path = /usr/share/revfe/plugins/
unix:INSTALLS += target
