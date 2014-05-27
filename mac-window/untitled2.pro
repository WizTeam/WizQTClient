#-------------------------------------------------
#
# Project created by QtCreator 2014-05-20T12:07:41
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = untitled2
TEMPLATE = app


SOURCES += main.cpp

HEADERS  += dialog.h

FORMS    += dialog.ui

LIBS += -framework Cocoa -framework Carbon

OBJECTIVE_SOURCES += \
    dialog.mm\
INAppStoreWindow/INAppStoreWindow.m\
INAppStoreWindow/INWindowButton.m\

