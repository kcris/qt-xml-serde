QT += core xml
QT -= gui

TARGET = example
CONFIG += console c++14
CONFIG -= app_bundle

TEMPLATE = app

# Include runtime headers
INCLUDEPATH += ../runtime

# Link runtime library
LIBS += -L.. -lxsdqt-runtime

SOURCES += \
    example.cpp \
    polymorphic_examples.cpp

HEADERS +=
