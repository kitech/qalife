#-------------------------------------------------
#
# Project created by QtCreator 2011-09-20T01:06:54
#
#-------------------------------------------------

QT       += core gui

TARGET = iamgod
TEMPLATE = app

QMAKE_CFLAGS += -pg
QMAKE_CXXFLAGS += -pg
QMAKE_LFLAGS += -pg

SOURCES += main.cpp\
        mainwindow.cpp \
    simplelog.cpp

HEADERS  += mainwindow.h \
    simplelog.h

FORMS    += mainwindow.ui

RESOURCES += \
    iamgod.qrc



