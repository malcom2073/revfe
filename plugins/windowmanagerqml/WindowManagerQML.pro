######################################################################
# Automatically generated by qmake (2.01a) Mon Sep 22 12:29:04 2008
######################################################################

TEMPLATE = lib
TARGET = WindowManagerQMLPlugin 
DEPENDPATH += .
INCLUDEPATH += . ../../CarPal /home/michael/libosmscout/libosmscout/include /home/michael/libosmscout/libosmscout-map-qt/include /home/michael/libosmscout/libosmscout-map/include
CONFIG += plugin debug
LIBS +=
win32:DESTDIR = ../../CarPal/debug/plugins/
unix:DESTDIR = ../../CarPal/plugins/
QT += xml
LIBS += -L/home/michael/libosmscout/libosmscout/src/.libs -L/home/michael/libosmscout/libosmscout-map/src/.libs -L/home/michael/libosmscout/libosmscout/src/.libs -L/home/michael/libosmscout/libosmscout-map-qt/src/.libs -L/home/michael/libosmscout/libosmscout-map/src/.libs -L/home/michael/libosmscout/libosmscout/src/.libs -losmscout -losmscoutmap -losmscoutmapqt

QT += declarative
QT += opengl
CONFIG += embed_manifest_exe
# Input
HEADERS += WindowManager.h \
           ../../CarPal/WindowInterface.h \
           ../../CarPal/BaseInterface.h \
    Window.h \
    ComplexModel.h \
    mapview.h
SOURCES += WindowManager.cpp \
           ../../CarPal/IPCMessage.cpp \ 
    Window.cpp \
    ComplexModel.cpp \
    mapview.cpp
unix:target.path = /usr/share/revfe/plugins/
unix:INSTALLS += target
