#-------------------------------------------------
#
# Project created by QtCreator 2011-11-20T17:04:40
#
#-------------------------------------------------

QT       += core
QT       -= gui

TARGET = Rpc
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
QT += network
CONFIG += debug
SOURCES += main.cpp \
    rpc.cpp \
    server.cpp \
    client.cpp

HEADERS += \
    rpc.h \
    server.h \
    client.h
