#-------------------------------------------------
#
# Project created by QtCreator 2015-05-31T12:53:53
#
#-------------------------------------------------
QMAKE_CXXFLAGS += -std=c++11

QT       -= gui

TARGET = QLogger
TEMPLATE = lib
CONFIG += staticlib

include(qlogger.pri)

unix {
    target.path = /usr/lib
    INSTALLS += target
}
