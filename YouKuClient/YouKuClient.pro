#-------------------------------------------------
#
# Project created by QtCreator 2014-12-30T14:14:23
#
#-------------------------------------------------

QT       += core gui network webkitwidgets sql

QMAKE_CXXFLAGS = -std=c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = YouKuClient
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    network.cpp \
    videomessage.cpp \
    searchbox.cpp \
    youkusuggest.cpp \
    mysqloperate.cpp

HEADERS  += mainwindow.h \
    network.h \
    videomessage.h \
    searchbox.h \
    youkusuggest.h \
    remoterequest.h \
    mysqloperate.h

RESOURCES += \
    res.qrc
