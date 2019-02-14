QT += core websockets
QT -= gui

CONFIG += c++11

TARGET = LoRaPackConverter
CONFIG += console
CONFIG -= app_bundle

DESTDIR = ../bin

TEMPLATE = app

DEFINES += REVISION_VER=$$system(hg id -n -r \"ancestors(.) and tag()\")

win32 {
    RC_FILE = main_icon.rc
}

unix {
    LIBS += -Wl,-rpath .
    QMAKE_LFLAGS += -no-pie
}

DEFINES += BUILD_DATE=__DATE__

SOURCES += main.cpp \
    packparser.cpp \
    cleanexit.cpp \
    webapiserver.cpp \
    net868client.cpp \
    converter.cpp \
    crc32.c \
    settings.cpp

DISTFILES +=

HEADERS += \
    packparser.h \
    cleanexit.h \
    webapiserver.h \
    net868client.h \
    converter.h \
    crc32.h \
    settings.h \
    appinfo.h
