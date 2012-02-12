TEMPLATE = lib
TARGET = RevFusionBrainPlugin
DEPENDPATH += .
INCLUDEPATH += . ../../revfe /usr/include/libusb-1.0/
CONFIG += plugin
unix:LIBS += -lusb-1.0
win32:DESTDIR = ../../revfe/plugins/
unix:DESTDIR = ../../revfe/plugins/
CONFIG += embed_manifest_exe
# Input
HEADERS += revfusionbrain.h fb.h ../../revfe/baseinterface.h
SOURCES += revfusionbrain.cpp fb.cpp ../../revfe/ipcmessage.cpp
unix:target.path = /usr/share/revfe/plugins/
unix:INSTALLS += target

