QT += core xml
QT -= gui

TARGET = xsd2cpp
CONFIG += console c++14
CONFIG -= app_bundle

TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    generator/main.cpp \
    generator/XsdParser.cpp \
    generator/CodeGenerator.cpp

HEADERS += \
    generator/XsdParser.h \
    generator/CodeGenerator.h

# Installation
unix {
    target.path = /usr/local/bin
    INSTALLS += target
}

win32 {
    target.path = $$[QT_INSTALL_BINS]
    INSTALLS += target
}
