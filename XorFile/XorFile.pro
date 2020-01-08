#-------------------------------------------------
#
# Project created by QtCreator 2014-11-02T16:49:01
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = XorFile
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    hashcalcmd5.cpp \
    appcore.cpp \
    xoraction.cpp \
    filepacket.cpp \
    filepacketedit.cpp \
    filedir.cpp \
    eden.cpp

HEADERS  += mainwindow.h \
    hashcalcmd5.h \
    appcore.h \
    xoraction.h \
    filepacket.h \
    filepacketedit.h \
    filedir.h \
    binary.h \
    eden.h \
    objmem.h

FORMS    += mainwindow.ui

CONFIG += c++11

DISTFILES +=

QMAKE_LFLAGS += -Wl,--large-address-aware
