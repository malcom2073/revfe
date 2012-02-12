######################################################################
# Automatically generated by qmake (2.01a) Mon Sep 22 12:29:04 2008
######################################################################

TEMPLATE = lib
TARGET = RevMediaLibraryPlugin
DEPENDPATH += .
INCLUDEPATH += . ../../revfe /usr/include/taglib C:\libs\taglib\include\taglib C:\libs\taglib\include\taglib\mpeg\id3v2\frames C:\libs\taglib\include\taglib\mpeg\id3v2 C:\libs\taglib\include\taglib\mpeg C:\libs\taglib\include\taglib\mpeg\id3v1
CONFIG += plugin debug
win32:DESTDIR = ../../revfe/plugins/
unix:DESTDIR = ../../revfe/plugins/
CONFIG += embed_manifest_exe
win32:LIBS += -lC:\libs\sqlite3\lib\sqlite3 -lC:\libs\taglib\lib\tag
unix:LIBS += -ltag 
QT += sql
# Input
HEADERS += revmedialibrary.h mediadb.h mediafileclass.h mediascannerclass.h mediascannerthread.h playlistclass.h id3tag.h mediadbthread.h ../../revfe/baseinterface.h ../../revfe/playlist.h ../../revfe/mediafile.h ../../revfe/standardmodel.h \
    videodb.h
SOURCES += revmedialibrary.cpp mediadb.cpp mediafileclass.cpp mediascannerclass.cpp mediascannerthread.cpp playlistclass.cpp id3tag.cpp mediadbthread.cpp ../../revfe/ipcmessage.cpp ../../revfe/playlist.cpp ../../revfe/mediafile.cpp ../../revfe/standardmodel.cpp \
    videodb.cpp
unix:target.path = /usr/share/revfe/plugins/
unix:INSTALLS += target
